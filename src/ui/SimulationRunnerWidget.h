#pragma once

#include <QWidget>

class QProcess;
class QPushButton;
class QProgressBar;
class QTextEdit;
class QLabel;
class QLineEdit;
class RunSession;
class ParameterSet;

class SimulationRunnerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationRunnerWidget(QWidget* parent = nullptr);

    void setParameterSet(ParameterSet* params);

    RunSession* runSession() const { return session_; }

signals:
    void simulationFinished(bool success, const QString& outputDir);

public slots:
    void launch();
    void cancel();

private slots:
    void onProcessStarted();
    void onProcessOutput();
    void onProcessFinished(int exitCode);
    void onProcessError();

private:
    void setupUI();
    void parseProgressLine(const QString& line);

    // Locate the simulator executable lazily (saved setting -> app-local
    // simulator/ -> $TSUNAMI_SIMULATOR_HOME -> sibling build -> PATH).
    // Returns an empty string if none is found; the UI still runs as a
    // standalone results viewer in that case.
    QString discoverSimulator() const;
    // Enable/disable the Launch button and update the status line based on
    // whether a valid simulator path is currently set.
    void updateLaunchState();

    ParameterSet* params_ = nullptr;
    RunSession* session_ = nullptr;
    QProcess* process_ = nullptr;

    QLineEdit* simPathEdit_ = nullptr;
    QPushButton* browseBtn_ = nullptr;
    QPushButton* launchBtn_ = nullptr;
    QPushButton* cancelBtn_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QTextEdit* logView_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};
