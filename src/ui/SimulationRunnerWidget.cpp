#include "SimulationRunnerWidget.h"
#include "model/RunSession.h"
#include "model/ParameterSet.h"

#include <QProcess>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDateTime>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>

SimulationRunnerWidget::SimulationRunnerWidget(QWidget* parent)
    : QWidget(parent),
      session_(new RunSession(this)),
      process_(new QProcess(this))
{
    setupUI();

    connect(process_, &QProcess::started,
            this, &SimulationRunnerWidget::onProcessStarted);
    connect(process_, &QProcess::readyReadStandardOutput,
            this, &SimulationRunnerWidget::onProcessOutput);
    connect(process_, &QProcess::readyReadStandardError,
            this, &SimulationRunnerWidget::onProcessOutput);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });
    connect(process_, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError) { onProcessError(); });

    // Try to locate the simulator without requiring it: the UI is usable as a
    // standalone results viewer even when no simulator is installed.
    const QString sim = discoverSimulator();
    if (!sim.isEmpty())
        simPathEdit_->setText(sim);
    updateLaunchState();
}

void SimulationRunnerWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Simulator path
    auto* pathRow = new QHBoxLayout;
    pathRow->addWidget(new QLabel(tr("Simulator:")));
    simPathEdit_ = new QLineEdit;
    simPathEdit_->setPlaceholderText(tr("Path to simulator executable (optional — viewing works without it)..."));
    connect(simPathEdit_, &QLineEdit::textChanged, this, [this](const QString&) { updateLaunchState(); });
    pathRow->addWidget(simPathEdit_, 1);
    browseBtn_ = new QPushButton(tr("Browse..."));
    connect(browseBtn_, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Select Simulator"), QString(),
            tr("Executables (*.exe);;All Files (*)"));
        if (!path.isEmpty())
            simPathEdit_->setText(path);
    });
    pathRow->addWidget(browseBtn_);
    layout->addLayout(pathRow);

    // Control buttons
    auto* btnRow = new QHBoxLayout;
    launchBtn_ = new QPushButton(tr("Launch Simulation"));
    connect(launchBtn_, &QPushButton::clicked, this, &SimulationRunnerWidget::launch);
    btnRow->addWidget(launchBtn_);

    cancelBtn_ = new QPushButton(tr("Cancel"));
    cancelBtn_->setEnabled(false);
    connect(cancelBtn_, &QPushButton::clicked, this, &SimulationRunnerWidget::cancel);
    btnRow->addWidget(cancelBtn_);

    btnRow->addStretch();
    layout->addLayout(btnRow);

    // Status and progress
    statusLabel_ = new QLabel(tr("Status: Idle"));
    layout->addWidget(statusLabel_);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    layout->addWidget(progressBar_);

    // Log view
    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    logView_->setFont(QFont("Consolas", 9));
    layout->addWidget(logView_, 1);
}

void SimulationRunnerWidget::setParameterSet(ParameterSet* params)
{
    params_ = params;
}

void SimulationRunnerWidget::launch()
{
    QString simPath = simPathEdit_->text().trimmed();
    if (simPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select a simulator executable."));
        return;
    }
    if (!QFileInfo::exists(simPath)) {
        QMessageBox::warning(this, tr("Error"), tr("Simulator not found: %1").arg(simPath));
        return;
    }

    logView_->clear();
    progressBar_->setValue(0);

    session_->setState(RunSession::State::Running);
    session_->setStartTime(QDateTime::currentDateTime());
    if (params_)
        session_->setTotalTimesteps(params_->timesteps());

    // Remember the working simulator for next time.
    QSettings().setValue(QStringLiteral("simulator/path"), simPath);

    // Launch process with working directory of the simulator
    process_->setWorkingDirectory(QFileInfo(simPath).absolutePath());
    process_->start(simPath, QStringList());

    launchBtn_->setEnabled(false);
    cancelBtn_->setEnabled(true);
    statusLabel_->setText(tr("Status: Running..."));
}

void SimulationRunnerWidget::cancel()
{
    if (process_->state() != QProcess::NotRunning) {
        session_->setState(RunSession::State::Cancelled);
        statusLabel_->setText(tr("Status: Cancelling..."));
        process_->terminate();
        // Force kill after 3 seconds — connect to this specific process lifecycle
        auto* killTimer = new QTimer(this);
        killTimer->setSingleShot(true);
        connect(killTimer, &QTimer::timeout, this, [this, killTimer]() {
            killTimer->deleteLater();
            if (process_->state() != QProcess::NotRunning)
                process_->kill();
        });
        // Cancel the timer if process finishes naturally
        connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                killTimer, &QTimer::stop);
        killTimer->start(3000);
    }
}

void SimulationRunnerWidget::onProcessStarted()
{
    session_->appendLog("Simulation started.\n");
}

void SimulationRunnerWidget::onProcessOutput()
{
    // Read stdout
    QByteArray stdOut = process_->readAllStandardOutput();
    if (!stdOut.isEmpty()) {
        QString text = QString::fromUtf8(stdOut);
        logView_->append(text);
        session_->appendLog(text);

        // Parse progress from each line
        const auto lines = text.split('\n');
        for (const QString& line : lines)
            parseProgressLine(line.trimmed());
    }

    // Read stderr
    QByteArray stdErr = process_->readAllStandardError();
    if (!stdErr.isEmpty()) {
        QString text = QString::fromUtf8(stdErr);
        logView_->append("<span style='color:red'>" + text.toHtmlEscaped() + "</span>");
        session_->appendLog(text);
    }
}

void SimulationRunnerWidget::onProcessFinished(int exitCode)
{
    session_->setExitCode(exitCode);
    session_->setEndTime(QDateTime::currentDateTime());

    launchBtn_->setEnabled(true);
    cancelBtn_->setEnabled(false);

    if (session_->state() == RunSession::State::Cancelled) {
        statusLabel_->setText(tr("Status: Cancelled (exit code %1)").arg(exitCode));
        // Still save manifest and emit signal for partial results
        QString outputDir = process_->workingDirectory() + "/Result";
        session_->setOutputDirectory(outputDir);
        session_->saveManifest(outputDir + "/run_manifest.json");
        emit simulationFinished(false, outputDir);
        return;
    }

    bool success = (exitCode == 0);
    session_->setState(success ? RunSession::State::Finished : RunSession::State::Failed);
    statusLabel_->setText(success
        ? tr("Status: Finished successfully")
        : tr("Status: Failed (exit code %1)").arg(exitCode));

    if (success)
        progressBar_->setValue(100);

    // Save run manifest
    QString outputDir = process_->workingDirectory() + "/Result";
    session_->setOutputDirectory(outputDir);
    session_->saveManifest(outputDir + "/run_manifest.json");

    emit simulationFinished(success, outputDir);
}

void SimulationRunnerWidget::onProcessError()
{
    session_->setState(RunSession::State::Failed);
    statusLabel_->setText(tr("Status: Error - %1").arg(process_->errorString()));
    launchBtn_->setEnabled(true);
    cancelBtn_->setEnabled(false);
}

void SimulationRunnerWidget::parseProgressLine(const QString& line)
{
    // Try to match "timestep N" or "Timestep: N"
    static QRegularExpression re(R"(timestep\s*:?\s*(\d+))", QRegularExpression::CaseInsensitiveOption);
    auto match = re.match(line);
    if (match.hasMatch()) {
        int ts = match.captured(1).toInt();
        session_->setCurrentTimestep(ts);

        int total = session_->totalTimesteps();
        if (total > 0) {
            int pct = std::min(99, ts * 100 / total);
            progressBar_->setValue(pct);
        }
    }
}

QString SimulationRunnerWidget::discoverSimulator() const
{
    const QString exe = QStringLiteral("MacCormackSimulation.exe");

    // 1. Previously saved / user-chosen path.
    const QString saved = QSettings().value(QStringLiteral("simulator/path")).toString();
    if (!saved.isEmpty() && QFileInfo::exists(saved))
        return saved;

    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates;
    // 2. Bundled next to the UI (app-local).
    candidates << appDir + "/simulator/" + exe
               << appDir + "/" + exe;
    // 3. Explicit environment override.
    const QByteArray home = qgetenv("TSUNAMI_SIMULATOR_HOME");
    if (!home.isEmpty()) {
        const QString h = QString::fromLocal8Bit(home);
        candidates << h + "/" + exe
                   << h + "/MacCormackSimulation/x64/Release/" + exe;
    }
    // 4. Sibling tsunami-simulator build (workspace convention). Walk UP from
    //    the app dir, because the UI exe usually sits under build/<config>/, so
    //    the workspace root with the sibling repo is several levels above.
    {
        QDir d(appDir);
        for (int up = 0; up < 6; ++up) {
            const QString base = d.absolutePath() + "/tsunami-simulator/MacCormackSimulation/x64/";
            candidates << base + "Release/" + exe
                       << base + "Debug/" + exe;
            if (!d.cdUp())
                break;
        }
    }

    for (const QString& c : candidates) {
        const QFileInfo fi(c);
        if (fi.exists() && fi.isFile())
            return fi.absoluteFilePath();
    }

    // 5. On PATH.
    const QString onPath = QStandardPaths::findExecutable(QStringLiteral("MacCormackSimulation"));
    if (!onPath.isEmpty())
        return onPath;

    return QString();  // none found — viewer-only mode (no error)
}

void SimulationRunnerWidget::updateLaunchState()
{
    // Never block startup or results viewing: only the Launch action depends
    // on the simulator being present.
    if (process_->state() != QProcess::NotRunning)
        return;  // a run is in progress; the process handlers own the buttons

    const QString p = simPathEdit_->text().trimmed();
    const bool ready = !p.isEmpty() && QFileInfo::exists(p);
    launchBtn_->setEnabled(ready);
    if (ready)
        statusLabel_->setText(tr("Status: Idle (simulator ready)"));
    else if (p.isEmpty())
        statusLabel_->setText(tr("Status: Idle — simulator not configured (results viewing available)"));
    else
        statusLabel_->setText(tr("Status: Idle — simulator not found at the given path"));
}
