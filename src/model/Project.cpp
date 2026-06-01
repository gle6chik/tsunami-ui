#include "Project.h"
#include "ParameterSet.h"
#include "GridDataset.h"
#include "ResultDataset.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Project::Project(QObject* parent)
    : QObject(parent),
      params_(new ParameterSet(this)),
      grid_(new GridDataset(this)),
      results_(new ResultDataset(this))
{
    connect(params_, &ParameterSet::parameterChanged, this, &Project::markDirty);
}

bool Project::loadProject(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return false;

    QJsonObject obj = doc.object();
    parFilePath_ = obj.value("parFile").toString();
    bathymetryPath_ = obj.value("bathymetry").toString();
    resultDir_ = obj.value("resultDir").toString();
    simulatorPath_ = obj.value("simulator").toString();
    projectFilePath_ = path;

    if (!parFilePath_.isEmpty())
        params_->loadFromFile(parFilePath_);

    if (!bathymetryPath_.isEmpty()) {
        if (bathymetryPath_.endsWith(".asc", Qt::CaseInsensitive))
            grid_->loadASC(bathymetryPath_);
        else
            grid_->loadZ(bathymetryPath_);
    }

    if (!resultDir_.isEmpty())
        results_->scanDirectory(resultDir_);

    dirty_ = false;
    emit projectChanged();
    return true;
}

bool Project::saveProject(const QString& path) const
{
    QJsonObject obj;
    obj["parFile"] = parFilePath_;
    obj["bathymetry"] = bathymetryPath_;
    obj["resultDir"] = resultDir_;
    obj["simulator"] = simulatorPath_;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(obj).toJson());
    return true;
}

void Project::setParFilePath(const QString& p) { parFilePath_ = p; markDirty(); }
void Project::setBathymetryPath(const QString& p) { bathymetryPath_ = p; markDirty(); }
void Project::setResultDirectory(const QString& p) { resultDir_ = p; markDirty(); }
void Project::setSimulatorPath(const QString& p) { simulatorPath_ = p; markDirty(); }

void Project::markDirty()
{
    if (!dirty_) {
        dirty_ = true;
        emit dirtyChanged(true);
    }
    emit projectChanged();
}
