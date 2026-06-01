#include "AsyncGridLoader.h"
#include "ASCParser.h"
#include "ZDataParser.h"

#include <QMetaObject>

AsyncGridLoader::AsyncGridLoader(GridDataset* dataset, const QString& path,
                                 FileType type, QObject* parent)
    : QThread(parent), dataset_(dataset), path_(path), type_(type)
{
}

void AsyncGridLoader::run()
{
    emit loadProgress(tr("Loading grid: %1").arg(path_));

    try {
        GridDataset::ParsedGrid parsed;

        if (type_ == ASC) {
            ASCParser parser;
            parser.Read(path_.toStdString());

            parsed.rows = parser.GetRows();
            parsed.cols = parser.GetCols();
            parsed.georeferenced = true;
            parsed.xllCorner = parser.GetXllCorner();
            parsed.yllCorner = parser.GetYllCorner();
            parsed.cellSize = parser.GetCellSize();
            parsed.nodataValue = parser.GetNoDataValue();

            parsed.data.resize(parsed.rows);
            for (int r = 0; r < parsed.rows; ++r) {
                parsed.data[r].resize(parsed.cols);
                for (int c = 0; c < parsed.cols; ++c)
                    parsed.data[r][c] = parser.GetValue(r, c);
            }
        } else {
            ZDataParser parser;
            parser.Read(path_.toStdString());

            parsed.rows = parser.GetRows();
            parsed.cols = parser.GetCols();
            if (parsed.rows == 0 || parsed.cols == 0) {
                emit loadFinished(false);
                return;
            }

            parsed.georeferenced = false;
            parsed.data.resize(parsed.rows);
            for (int r = 0; r < parsed.rows; ++r) {
                parsed.data[r].resize(parsed.cols);
                for (int c = 0; c < parsed.cols; ++c)
                    parsed.data[r][c] = parser.GetValue(r, c);
            }
        }

        // Transfer parsed data to main thread safely via queued invocation
        auto* parsedPtr = new GridDataset::ParsedGrid(std::move(parsed));
        QMetaObject::invokeMethod(dataset_, [this, parsedPtr]() {
            dataset_->applyParsedData(std::move(*parsedPtr));
            delete parsedPtr;
            emit loadFinished(true);
        }, Qt::QueuedConnection);

    } catch (const std::exception&) {
        emit loadFinished(false);
    }
}
