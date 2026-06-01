#pragma once

#include <cmath>
#include <utility>

// WGS84 <-> Web Mercator (EPSG:3857) conversion utilities
namespace Reprojection
{

constexpr double EARTH_RADIUS = 6378137.0; // WGS84 semi-major axis in meters
constexpr double MAX_LAT = 85.06;

// WGS84 (lon, lat) in degrees -> Web Mercator (x, y) in meters
inline std::pair<double, double> lonLatToMercator(double lon, double lat)
{
    double x = EARTH_RADIUS * lon * M_PI / 180.0;
    double latRad = std::clamp(lat, -MAX_LAT, MAX_LAT) * M_PI / 180.0;
    double y = EARTH_RADIUS * std::log(std::tan(M_PI / 4.0 + latRad / 2.0));
    return {x, y};
}

// Web Mercator (x, y) in meters -> WGS84 (lon, lat) in degrees
inline std::pair<double, double> mercatorToLonLat(double x, double y)
{
    double lon = x / EARTH_RADIUS * 180.0 / M_PI;
    double lat = (2.0 * std::atan(std::exp(y / EARTH_RADIUS)) - M_PI / 2.0) * 180.0 / M_PI;
    return {lon, lat};
}

// Tile coordinates for a given (lon, lat) and zoom level
inline std::pair<int, int> lonLatToTile(double lon, double lat, int zoom)
{
    int n = 1 << zoom;
    double latRad = std::clamp(lat, -MAX_LAT, MAX_LAT) * M_PI / 180.0;
    int tx = static_cast<int>((lon + 180.0) / 360.0 * n);
    int ty = static_cast<int>((1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * n);
    tx = std::clamp(tx, 0, n - 1);
    ty = std::clamp(ty, 0, n - 1);
    return {tx, ty};
}

// Top-left corner of a tile in (lon, lat) degrees
inline std::pair<double, double> tileToLonLat(int tx, int ty, int zoom)
{
    int n = 1 << zoom;
    double lon = static_cast<double>(tx) / n * 360.0 - 180.0;
    double latRad = std::atan(std::sinh(M_PI * (1.0 - 2.0 * static_cast<double>(ty) / n)));
    double lat = latRad * 180.0 / M_PI;
    return {lon, lat};
}

} // namespace Reprojection
