#pragma once

#include <QMainWindow>

class QTabWidget;
class ParameterEditorWidget;
class GridViewerWidget;
class SimulationRunnerWidget;
class Project;
class TileProvider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void setupMenuBar();
    void setupTabs();
    void connectModules();

    void loadBathymetry(const QString& path);
    void onSimulationFinished(bool success, const QString& outputDir);

    QTabWidget* tabs_ = nullptr;
    Project* project_ = nullptr;
    TileProvider* tileProvider_ = nullptr;

    ParameterEditorWidget* paramEditor_ = nullptr;
    GridViewerWidget* gridViewer_ = nullptr;
    SimulationRunnerWidget* simRunner_ = nullptr;
};
