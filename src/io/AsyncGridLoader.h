#pragma once

#include <QThread>
#include <QString>
#include "model/GridDataset.h"

class AsyncGridLoader : public QThread
{
    Q_OBJECT

public:
    enum FileType { ASC, Z };

    AsyncGridLoader(GridDataset* dataset, const QString& path,
                    FileType type, QObject* parent = nullptr);

signals:
    void loadFinished(bool success);
    void loadProgress(const QString& message);

protected:
    void run() override;

private:
    GridDataset* dataset_;
    QString path_;
    FileType type_;
};
