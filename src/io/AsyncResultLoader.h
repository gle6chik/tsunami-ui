#pragma once

#include <QThread>
#include <QMutex>
#include <QString>
#include <QList>
#include <memory>

struct FrameData;
class ResultDataset;

class AsyncResultLoader : public QThread
{
    Q_OBJECT

public:
    AsyncResultLoader(ResultDataset* dataset, QObject* parent = nullptr);

    // Request prefetch of frames around current position
    void requestPrefetch(int currentTimestep, int radius = 5);
    void requestStop();

signals:
    void framePrefetched(int timestep);

protected:
    void run() override;

private:
    ResultDataset* dataset_;
    QList<int> requestedTimesteps_;
    bool stopRequested_ = false;
    mutable QMutex mutex_;
};
