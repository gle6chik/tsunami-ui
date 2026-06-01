#pragma once

#include <QString>
#include <vector>

// Binary cache for fast re-reads of text result files.
// Format: 16-byte header (int32 rows, int32 cols, 8 bytes reserved)
//         followed by rows*cols doubles in row-major order.
class BinaryCache
{
public:
    static QString cachePath(const QString& textFilePath);

    static bool hasCachedVersion(const QString& textFilePath);

    // Write binary cache from parsed data
    static bool write(const QString& textFilePath,
                      int rows, int cols,
                      const std::vector<double>& values);

    // Read binary cache; returns false if cache missing or invalid
    static bool read(const QString& textFilePath,
                     int& rows, int& cols,
                     std::vector<double>& values);
};
