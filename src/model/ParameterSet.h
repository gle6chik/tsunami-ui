#pragma once

#include <QString>
#include <QObject>
#include <vector>
#include <string>

// Forward declarations - we wrap the simulator types
#include "CoordSystem.h"

struct SourceParams {
    QString name;
    int type = 0; // 0=Ellipsoidal
    double centerX = 0.0;
    double centerY = 0.0;
    double radius1 = 0.0;
    double radius2 = 0.0;
    double angle = 0.0;
};

class ParameterSet : public QObject
{
    Q_OBJECT

public:
    explicit ParameterSet(QObject* parent = nullptr);

    // Load/save .par files
    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path) const;

    // Accessors
    double deltaT() const { return deltaT_; }
    void setDeltaT(double v);

    int timesteps() const { return timesteps_; }
    void setTimesteps(int v);

    int numThreads() const { return numThreads_; }
    void setNumThreads(int v);

    int numSubgrids() const { return numSubgrids_; }
    void setNumSubgrids(int v);

    int boundaryExchangeType() const { return boundaryExchangeType_; }
    void setBoundaryExchangeType(int v);

    int hatBoundaryExchangeType() const { return hatBoundaryExchangeType_; }
    void setHatBoundaryExchangeType(int v);

    int deltaXType() const { return deltaXType_; }
    void setDeltaXType(int v);

    double deltaXStep() const { return deltaXStep_; }
    void setDeltaXStep(double v);

    int deltaYType() const { return deltaYType_; }
    void setDeltaYType(int v);

    double deltaYStep() const { return deltaYStep_; }
    void setDeltaYStep(double v);

    int inputBathymetryGenerationType() const { return inputBathGenType_; }
    void setInputBathymetryGenerationType(int v);

    int inputBathymetryType() const { return inputBathType_; }
    void setInputBathymetryType(int v);

    double minimumDepthToCalculate() const { return minDepth_; }
    void setMinimumDepthToCalculate(double v);

    CoordSystem coordSystem() const { return coordSystem_; }
    void setCoordSystem(CoordSystem v);

    const std::vector<SourceParams>& sources() const { return sources_; }
    void setSources(const std::vector<SourceParams>& sources);
    void addSource(const SourceParams& s);
    void removeSource(int index);

    // CFL check: returns CFL number given max depth
    double computeCFL(double maxDepth) const;
    bool isCFLValid(double maxDepth) const;

    // Generation type
    QString generationType() const { return generationType_; }
    void setGenerationType(const QString& v);

signals:
    void parameterChanged();

private:
    double deltaT_ = 0.5;
    int timesteps_ = 1000;
    int numThreads_ = 8;
    int numSubgrids_ = 8;
    int boundaryExchangeType_ = 0;
    int hatBoundaryExchangeType_ = 0;
    int deltaXType_ = 0;
    double deltaXStep_ = 1000.0;
    int deltaYType_ = 0;
    double deltaYStep_ = 1000.0;
    QString generationType_ = "1";
    int inputBathGenType_ = 1;
    int inputBathType_ = 1;
    double minDepth_ = 7.0;
    CoordSystem coordSystem_ = CoordSystem::Cartesian;
    std::vector<SourceParams> sources_;
};
