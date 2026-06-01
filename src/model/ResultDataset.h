#pragma once

#include <QObject>
#include <QMutex>
#include <QString>
#include <QMap>
#include <vector>
#include <memory>

struct FrameData {
    int timestep = 0;
    int rows = 0;
    int cols = 0;
    std::vector<double> values; // row-major
    double minVal = 0.0;
    double maxVal = 0.0;

    double value(int r, int c) const {
        if (r < 0 || r >= rows || c < 0 || c >= cols) return 0.0;
        return values[r * cols + c];
    }
};

class ResultDataset : public QObject
{
    Q_OBJECT

public:
    explicit ResultDataset(QObject* parent = nullptr);

    // Scan directory for result files (eta_timestep_*.txt or any *.txt)
    bool scanDirectory(const QString& dirPath);

    // Load a single result file as frame 0
    bool loadSingleFile(const QString& filePath);

    bool isLoaded() const { return !frameIndex_.isEmpty(); }
    int frameCount() const { return frameIndex_.size(); }
    QList<int> timesteps() const { return frameIndex_.keys(); }

    // Get frame (loads from disk or cache)
    std::shared_ptr<FrameData> frame(int timestep);

    // LRU cache config
    int cacheCapacity() const { return cacheCapacity_; }
    void setCacheCapacity(int n) { cacheCapacity_ = n; }

    QString directory() const { return dirPath_; }

    // If simulator writes results transposed (nx-major), set expected grid dims
    // so loadFrame can auto-transpose when needed
    void setExpectedGridDims(int gridRows, int gridCols);
    bool needsTranspose() const { return transposeResults_; }

    // Clear all loaded data
    void clear();

signals:
    void directoryScanned(int frameCount);
    void frameLoaded(int timestep);

private:
    std::shared_ptr<FrameData> loadFrame(const QString& path, int timestep);
    void evictCache();

    mutable QMutex mutex_;
    QString dirPath_;
    QMap<int, QString> frameIndex_; // timestep -> filepath
    QMap<int, std::shared_ptr<FrameData>> cache_;
    QList<int> cacheOrder_; // LRU order (most recent at back)
    int cacheCapacity_ = 50;
    bool transposeResults_ = false;
    int expectedGridRows_ = 0;
    int expectedGridCols_ = 0;
};
