#pragma once

#include <QObject>
#include <QString>
#include <vector>

// Stores loaded bathymetry grid data and metadata
class GridDataset : public QObject
{
    Q_OBJECT

public:
    explicit GridDataset(QObject* parent = nullptr);

    // Load from ASC or Z file (blocking call — use AsyncGridLoader for background)
    bool loadASC(const QString& path);
    bool loadZ(const QString& path);

    bool isLoaded() const { return loaded_; }
    void clear();

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    double value(int row, int col) const;
    double maxValue() const { return maxVal_; }
    double minValue() const { return minVal_; }

    // Georeferencing (valid only for ASC files)
    bool isGeoreferenced() const { return georeferenced_; }
    double xllCorner() const { return xllCorner_; }
    double yllCorner() const { return yllCorner_; }
    double cellSize() const { return cellSize_; }

    // Convert grid indices to world coordinates
    double worldX(int col) const;
    double worldY(int row) const;

    double nodataValue() const { return nodataValue_; }

    const std::vector<std::vector<double>>& data() const { return data_; }

    // Accept pre-parsed data (thread-safe transfer from worker)
    struct ParsedGrid {
        int rows = 0, cols = 0;
        std::vector<std::vector<double>> data;
        bool georeferenced = false;
        double xllCorner = 0, yllCorner = 0, cellSize = 0, nodataValue = -9999.0;
    };
    void applyParsedData(ParsedGrid&& parsed);

signals:
    void dataLoaded();

private:
    void computeMinMax();

    bool loaded_ = false;
    int rows_ = 0;
    int cols_ = 0;
    std::vector<std::vector<double>> data_;

    // Min/max values
    double minVal_ = 0.0;
    double maxVal_ = 0.0;

    // Geo metadata (ASC only)
    bool georeferenced_ = false;
    double xllCorner_ = 0.0;
    double yllCorner_ = 0.0;
    double cellSize_ = 0.0;
    double nodataValue_ = -9999.0;
};
