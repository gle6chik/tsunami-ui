#pragma once

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QNetworkAccessManager>
#include <QString>

// Fetches and caches OSM map tiles
class TileProvider : public QObject
{
    Q_OBJECT

public:
    explicit TileProvider(QObject* parent = nullptr);

    // Request a tile; returns cached pixmap or triggers download
    // Returns null QPixmap if not yet available (will emit tileReady later)
    QPixmap requestTile(int zoom, int tx, int ty);

    // Check if tile is in memory or disk cache
    bool hasTile(int zoom, int tx, int ty) const;

    // Cache directory
    QString cacheDir() const { return cacheDir_; }
    void setCacheDir(const QString& dir) { cacheDir_ = dir; }

signals:
    void tileReady(int zoom, int tx, int ty, const QPixmap& pixmap);

private:
    struct TileKey {
        int zoom, tx, ty;
        bool operator<(const TileKey& o) const {
            if (zoom != o.zoom) return zoom < o.zoom;
            if (tx != o.tx) return tx < o.tx;
            return ty < o.ty;
        }
    };

    QString tilePath(int zoom, int tx, int ty) const;
    QString tileUrl(int zoom, int tx, int ty) const;
    bool loadFromDisk(int zoom, int tx, int ty);
    void downloadTile(int zoom, int tx, int ty);

    QNetworkAccessManager* nam_ = nullptr;
    QMap<TileKey, QPixmap> memCache_;
    QMap<TileKey, bool> pending_; // tiles being downloaded
    QString cacheDir_;
};
