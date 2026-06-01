#include "AsyncResultLoader.h"
#include "model/ResultDataset.h"

#include <QMutexLocker>
#include <algorithm>

AsyncResultLoader::AsyncResultLoader(ResultDataset* dataset, QObject* parent)
    : QThread(parent), dataset_(dataset)
{
}

void AsyncResultLoader::requestPrefetch(int currentTimestep, int radius)
{
    QMutexLocker lock(&mutex_);
    requestedTimesteps_.clear();

    auto allTs = dataset_->timesteps();
    int idx = allTs.indexOf(currentTimestep);
    if (idx < 0) return;

    for (int i = std::max(0, idx - radius);
         i <= std::min(allTs.size() - 1, static_cast<qsizetype>(idx + radius)); ++i) {
        requestedTimesteps_.append(allTs[i]);
    }

    if (!isRunning())
        start();
}

void AsyncResultLoader::requestStop()
{
    QMutexLocker lock(&mutex_);
    stopRequested_ = true;
    requestedTimesteps_.clear();
}

void AsyncResultLoader::run()
{
    while (true) {
        int ts = -1;
        {
            QMutexLocker lock(&mutex_);
            if (stopRequested_ || requestedTimesteps_.isEmpty())
                break;
            ts = requestedTimesteps_.takeFirst();
        }

        // This will load from disk if not cached
        dataset_->frame(ts);
        emit framePrefetched(ts);
    }

    QMutexLocker lock(&mutex_);
    stopRequested_ = false;
}
