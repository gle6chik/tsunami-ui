#include "TileProvider.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QImage>

TileProvider::TileProvider(QObject* parent)
    : QObject(parent),
      nam_(new QNetworkAccessManager(this))
{
    cacheDir_ = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                + "/TsunamiSimUI/tiles";
}

QPixmap TileProvider::requestTile(int zoom, int tx, int ty)
{
    TileKey key{zoom, tx, ty};

    // Memory cache
    auto it = memCache_.find(key);
    if (it != memCache_.end())
        return it.value();

    // Disk cache
    if (loadFromDisk(zoom, tx, ty))
        return memCache_[key];

    // Download
    if (!pending_.contains(key))
        downloadTile(zoom, tx, ty);

    return QPixmap();
}

bool TileProvider::hasTile(int zoom, int tx, int ty) const
{
    TileKey key{zoom, tx, ty};
    if (memCache_.contains(key))
        return true;
    return QFile::exists(tilePath(zoom, tx, ty));
}

QString TileProvider::tilePath(int zoom, int tx, int ty) const
{
    return QString("%1/%2/%3/%4.png").arg(cacheDir_).arg(zoom).arg(tx).arg(ty);
}

QString TileProvider::tileUrl(int zoom, int tx, int ty) const
{
    return QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(zoom).arg(tx).arg(ty);
}

bool TileProvider::loadFromDisk(int zoom, int tx, int ty)
{
    QString path = tilePath(zoom, tx, ty);
    QPixmap pix;
    if (pix.load(path)) {
        memCache_[{zoom, tx, ty}] = pix;
        return true;
    }
    return false;
}

void TileProvider::downloadTile(int zoom, int tx, int ty)
{
    TileKey key{zoom, tx, ty};
    pending_[key] = true;

    QNetworkRequest req(QUrl(tileUrl(zoom, tx, ty)));
    req.setHeader(QNetworkRequest::UserAgentHeader, "TsunamiSimUI/1.0");

    QNetworkReply* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, key, zoom, tx, ty]() {
        reply->deleteLater();
        pending_.remove(key);

        if (reply->error() != QNetworkReply::NoError)
            return;

        QByteArray data = reply->readAll();
        QPixmap pix;
        if (!pix.loadFromData(data))
            return;

        // Save to disk cache
        QString path = tilePath(zoom, tx, ty);
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
            file.write(data);

        memCache_[key] = pix;
        emit tileReady(zoom, tx, ty, pix);
    });
}
