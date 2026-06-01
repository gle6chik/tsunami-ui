#include "MainWindow.h"
#include "ParameterEditorWidget.h"
#include "GridViewerWidget.h"
#include "GridScene.h"
#include "SimulationRunnerWidget.h"
#include "model/Project.h"
#include "model/ParameterSet.h"
#include "model/GridDataset.h"
#include "model/ResultDataset.h"
#include "geo/TileProvider.h"
#include "io/AsyncGridLoader.h"

#include <QTabWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      project_(new Project(this)),
      tileProvider_(new TileProvider(this))
{
    setWindowTitle(tr("Tsunami Simulator UI"));
    resize(1400, 900);

    setupMenuBar();
    setupTabs();
    connectModules();

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupMenuBar()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    auto* newProjectAction = fileMenu->addAction(tr("&New Project"));
    connect(newProjectAction, &QAction::triggered, this, [this]() {
        project_->deleteLater();
        project_ = new Project(this);
        connectModules();
        statusBar()->showMessage(tr("New project created"));
    });

    auto* openProjectAction = fileMenu->addAction(tr("Open &Project..."));
    connect(openProjectAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Open Project"), QString(),
            tr("Tsunami Project (*.tsunamiproj);;All Files (*)"));
        if (!path.isEmpty()) {
            project_->deleteLater();
            project_ = new Project(this);
            if (project_->loadProject(path)) {
                if (!project_->parFilePath().isEmpty())
                    paramEditor_->loadFromFile(project_->parFilePath());
                connectModules();
                statusBar()->showMessage(tr("Project loaded: %1").arg(path));
            } else {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to load project: %1").arg(path));
            }
        }
    });

    auto* saveProjectAction = fileMenu->addAction(tr("&Save Project..."));
    connect(saveProjectAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, tr("Save Project"), QString(),
            tr("Tsunami Project (*.tsunamiproj);;All Files (*)"));
        if (!path.isEmpty())
            project_->saveProject(path);
    });

    fileMenu->addSeparator();

    auto* openParAction = fileMenu->addAction(tr("Open .&par file..."));
    connect(openParAction, &QAction::triggered, this, [this]() {
        QSettings s;
        QString path = QFileDialog::getOpenFileName(
            this, tr("Open Parameter File"), s.value("lastParDir").toString(),
            tr("Parameter Files (*.par);;All Files (*)"));
        if (!path.isEmpty() && paramEditor_) {
            s.setValue("lastParDir", QFileInfo(path).absolutePath());
            paramEditor_->loadFromFile(path);
            project_->setParFilePath(path);
            // Update layer tree and reconnect params to scene
            auto* params = paramEditor_->parameterSet();
            gridViewer_->setParameterSet(params, QFileInfo(path).fileName());
            simRunner_->setParameterSet(params);
            if (project_->gridDataset()->isLoaded())
                gridViewer_->scene()->rebuildRaster();
        }
    });

    auto* saveParAction = fileMenu->addAction(tr("Sa&ve .par file..."));
    connect(saveParAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, tr("Save Parameter File"), QString(),
            tr("Parameter Files (*.par);;All Files (*)"));
        if (!path.isEmpty() && paramEditor_)
            paramEditor_->saveToFile(path);
    });

    fileMenu->addSeparator();

    auto* openBathAction = fileMenu->addAction(tr("Open &Bathymetry..."));
    connect(openBathAction, &QAction::triggered, this, [this]() {
        QSettings s;
        QString path = QFileDialog::getOpenFileName(
            this, tr("Open Bathymetry"), s.value("lastBathDir").toString(),
            tr("Grid Files (*.asc *.z);;ASC Files (*.asc);;Z Files (*.z);;All Files (*)"));
        if (!path.isEmpty()) {
            s.setValue("lastBathDir", QFileInfo(path).absolutePath());
            loadBathymetry(path);
        }
    });

    fileMenu->addSeparator();

    auto* exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::setupTabs()
{
    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    paramEditor_ = new ParameterEditorWidget(this);
    tabs_->addTab(paramEditor_, tr("Parameters"));

    gridViewer_ = new GridViewerWidget(this);
    tabs_->addTab(gridViewer_, tr("Map && Results"));

    simRunner_ = new SimulationRunnerWidget(this);
    tabs_->addTab(simRunner_, tr("Simulation"));
}

void MainWindow::connectModules()
{
    disconnect(simRunner_, &SimulationRunnerWidget::simulationFinished,
               this, &MainWindow::onSimulationFinished);

    auto* params = paramEditor_->parameterSet();
    gridViewer_->setParameterSet(params); // no filename = no tree item yet
    simRunner_->setParameterSet(params);

    gridViewer_->setGridDataset(project_->gridDataset()); // no filename = no tree item yet
    gridViewer_->setResultDataset(project_->resultDataset());
    gridViewer_->setTileProvider(tileProvider_);

    connect(simRunner_, &SimulationRunnerWidget::simulationFinished,
            this, &MainWindow::onSimulationFinished);

    connect(params, &ParameterSet::parameterChanged, this, [this]() {
        if (project_->gridDataset()->isLoaded())
            gridViewer_->scene()->rebuildRaster();
    }, static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
}

void MainWindow::loadBathymetry(const QString& path)
{
    project_->setBathymetryPath(path);

    auto* grid = project_->gridDataset();
    bool isASC = path.endsWith(".asc", Qt::CaseInsensitive);

    auto* loader = new AsyncGridLoader(
        grid, path,
        isASC ? AsyncGridLoader::ASC : AsyncGridLoader::Z,
        this);

    statusBar()->showMessage(tr("Loading bathymetry: %1...").arg(path));

    auto* currentProject = project_;
    connect(loader, &AsyncGridLoader::loadFinished, this,
            [this, loader, path, currentProject](bool ok) {
        loader->deleteLater();
        if (project_ != currentProject) return;

        if (ok) {
            gridViewer_->setGridDataset(project_->gridDataset(),
                                        QFileInfo(path).fileName());
            statusBar()->showMessage(tr("Loaded: %1 (%2x%3)")
                .arg(path)
                .arg(project_->gridDataset()->rows())
                .arg(project_->gridDataset()->cols()));
            tabs_->setCurrentWidget(gridViewer_);
        } else {
            statusBar()->showMessage(tr("Failed to load: %1").arg(path));
            QMessageBox::warning(this, tr("Error"),
                                 tr("Failed to load bathymetry file: %1").arg(path));
        }
    });

    loader->start();
}

void MainWindow::onSimulationFinished(bool success, const QString& outputDir)
{
    if (success) {
        auto reply = QMessageBox::question(
            this, tr("Simulation Finished"),
            tr("Simulation completed successfully.\nLoad results from %1?").arg(outputDir));

        if (reply == QMessageBox::Yes) {
            project_->setResultDirectory(outputDir);
            project_->resultDataset()->scanDirectory(outputDir);
            gridViewer_->setResultDataset(project_->resultDataset());
            gridViewer_->addResultLayer(QFileInfo(outputDir).fileName());
            tabs_->setCurrentWidget(gridViewer_);
        }
    }
}
