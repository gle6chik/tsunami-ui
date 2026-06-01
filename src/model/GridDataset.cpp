#include "GridDataset.h"

#include "ASCParser.h"
#include "ZDataParser.h"

#include <limits>
#include <stdexcept>
#include <utility>

GridDataset::GridDataset(QObject* parent)
    : QObject(parent)
{
}

void GridDataset::clear()
{
    loaded_ = false;
    rows_ = 0;
    cols_ = 0;
    data_.clear();
    minVal_ = 0.0;
    maxVal_ = 0.0;
    georeferenced_ = false;
    xllCorner_ = 0.0;
    yllCorner_ = 0.0;
    cellSize_ = 0.0;
}

bool GridDataset::loadASC(const QString& path)
{
    try {
        ASCParser parser;
        parser.Read(path.toStdString());

        int newRows = parser.GetRows();
        int newCols = parser.GetCols();

        std::vector<std::vector<double>> newData(newRows);
        for (int r = 0; r < newRows; ++r) {
            newData[r].resize(newCols);
            for (int c = 0; c < newCols; ++c) {
                newData[r][c] = parser.GetValue(r, c);
            }
        }

        // Commit only after all parsing succeeded
        rows_ = newRows;
        cols_ = newCols;
        data_ = std::move(newData);
        georeferenced_ = true;
        xllCorner_ = parser.GetXllCorner();
        yllCorner_ = parser.GetYllCorner();
        cellSize_ = parser.GetCellSize();
        nodataValue_ = parser.GetNoDataValue();

        computeMinMax();
        loaded_ = true;
        emit dataLoaded();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool GridDataset::loadZ(const QString& path)
{
    try {
        ZDataParser parser;
        parser.Read(path.toStdString());

        int newRows = parser.GetRows();
        int newCols = parser.GetCols();

        if (newRows == 0 || newCols == 0)
            return false;

        std::vector<std::vector<double>> newData(newRows);
        for (int r = 0; r < newRows; ++r) {
            newData[r].resize(newCols);
            for (int c = 0; c < newCols; ++c) {
                newData[r][c] = parser.GetValue(r, c);
            }
        }

        // Commit only after all parsing succeeded
        rows_ = newRows;
        cols_ = newCols;
        data_ = std::move(newData);
        georeferenced_ = false;
        nodataValue_ = -9999.0;

        computeMinMax();
        loaded_ = true;
        emit dataLoaded();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

double GridDataset::value(int row, int col) const
{
    if (row < 0 || row >= rows_ || col < 0 || col >= cols_)
        return 0.0;
    return data_[row][col];
}

double GridDataset::worldX(int col) const
{
    if (!georeferenced_) return static_cast<double>(col);
    return xllCorner_ + col * cellSize_;
}

double GridDataset::worldY(int row) const
{
    if (!georeferenced_) return static_cast<double>(row);
    // ASC format: row 0 is top (north), yllCorner is bottom
    return yllCorner_ + (rows_ - 1 - row) * cellSize_;
}

void GridDataset::applyParsedData(ParsedGrid&& parsed)
{
    rows_ = parsed.rows;
    cols_ = parsed.cols;
    data_ = std::move(parsed.data);
    georeferenced_ = parsed.georeferenced;
    xllCorner_ = parsed.xllCorner;
    yllCorner_ = parsed.yllCorner;
    cellSize_ = parsed.cellSize;
    nodataValue_ = parsed.nodataValue;

    computeMinMax();
    loaded_ = true;
    emit dataLoaded();
}

void GridDataset::computeMinMax()
{
    minVal_ = std::numeric_limits<double>::max();
    maxVal_ = std::numeric_limits<double>::lowest();

    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            double v = data_[r][c];
            // Skip NODATA values
            if (georeferenced_ && v == nodataValue_)
                continue;
            if (v < minVal_) minVal_ = v;
            if (v > maxVal_) maxVal_ = v;
        }
    }

    // Handle case where all values are NODATA or grid is empty
    if (minVal_ > maxVal_) {
        minVal_ = 0.0;
        maxVal_ = 0.0;
    }
}
