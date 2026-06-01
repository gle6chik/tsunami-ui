#include "BinaryCache.h"

#include <QFile>
#include <QFileInfo>
#include <QDataStream>

static constexpr quint32 MAGIC = 0x54534243; // "TSBC"
static constexpr quint32 VERSION = 1;

QString BinaryCache::cachePath(const QString& textFilePath)
{
    return textFilePath + ".bin";
}

bool BinaryCache::hasCachedVersion(const QString& textFilePath)
{
    QFileInfo binInfo(cachePath(textFilePath));
    QFileInfo txtInfo(textFilePath);
    return binInfo.exists() && binInfo.lastModified() >= txtInfo.lastModified();
}

bool BinaryCache::write(const QString& textFilePath,
                        int rows, int cols,
                        const std::vector<double>& values)
{
    if (static_cast<int>(values.size()) != rows * cols)
        return false;

    QFile file(cachePath(textFilePath));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::DoublePrecision);

    out << MAGIC << VERSION;
    out << static_cast<qint32>(rows) << static_cast<qint32>(cols);

    for (double v : values)
        out << v;

    return out.status() == QDataStream::Ok;
}

bool BinaryCache::read(const QString& textFilePath,
                       int& rows, int& cols,
                       std::vector<double>& values)
{
    if (!hasCachedVersion(textFilePath))
        return false;

    QFile file(cachePath(textFilePath));
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setFloatingPointPrecision(QDataStream::DoublePrecision);

    quint32 magic, ver;
    in >> magic >> ver;
    if (magic != MAGIC || ver != VERSION)
        return false;

    qint32 r, c;
    in >> r >> c;
    if (r <= 0 || c <= 0 || static_cast<qint64>(r) * c > 100'000'000)
        return false;

    rows = r;
    cols = c;
    values.resize(rows * cols);
    for (int i = 0; i < rows * cols; ++i)
        in >> values[i];

    return in.status() == QDataStream::Ok;
}
