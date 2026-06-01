#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

class ParameterSet;
class GridDataset;
class ResultDataset;

class Project : public QObject
{
    Q_OBJECT

public:
    explicit Project(QObject* parent = nullptr);

    // Load/save .tsunamiproj JSON
    bool loadProject(const QString& path);
    bool saveProject(const QString& path) const;

    // Paths
    QString parFilePath() const { return parFilePath_; }
    void setParFilePath(const QString& p);

    QString bathymetryPath() const { return bathymetryPath_; }
    void setBathymetryPath(const QString& p);

    QString resultDirectory() const { return resultDir_; }
    void setResultDirectory(const QString& p);

    QString simulatorPath() const { return simulatorPath_; }
    void setSimulatorPath(const QString& p);

    // Sub-models (owned)
    ParameterSet* parameterSet() const { return params_; }
    GridDataset* gridDataset() const { return grid_; }
    ResultDataset* resultDataset() const { return results_; }

    bool isDirty() const { return dirty_; }

signals:
    void projectChanged();
    void dirtyChanged(bool dirty);

private:
    void markDirty();

    ParameterSet* params_ = nullptr;
    GridDataset* grid_ = nullptr;
    ResultDataset* results_ = nullptr;

    QString parFilePath_;
    QString bathymetryPath_;
    QString resultDir_;
    QString simulatorPath_;
    QString projectFilePath_;
    bool dirty_ = false;
};
