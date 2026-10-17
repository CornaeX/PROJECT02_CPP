#include "GeoUtils.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
constexpr double Pi = 3.14159265358979323846;
double radians(double degrees) { return degrees * Pi / 180.0; }
}

namespace GeoUtils {
double haversineKm(double latitude1, double longitude1, double latitude2, double longitude2) {
    if (!isValidCoordinate(latitude1, longitude1) || !isValidCoordinate(latitude2, longitude2)) {
        throw std::invalid_argument("Haversine calculation received invalid coordinates.");
    }
    const double lat1 = radians(latitude1);
    const double lat2 = radians(latitude2);
    const double deltaLat = radians(latitude2 - latitude1);
    const double deltaLon = radians(longitude2 - longitude1);
    const double a = std::sin(deltaLat / 2.0) * std::sin(deltaLat / 2.0) +
                     std::cos(lat1) * std::cos(lat2) *
                     std::sin(deltaLon / 2.0) * std::sin(deltaLon / 2.0);
    const double clamped = std::max(0.0, std::min(1.0, a));
    return EarthRadiusKm * 2.0 * std::atan2(std::sqrt(clamped), std::sqrt(1.0 - clamped));
}

double straightLineDistanceKm(const Place& from, const Place& to) {
    return haversineKm(from.latitude, from.longitude, to.latitude, to.longitude);
}

std::string googleMapsSearchUrl(const Place& place) {
    std::ostringstream query;
    query << std::fixed << std::setprecision(6) << place.latitude << ',' << place.longitude;
    return "https://www.google.com/maps/search/?api=1&query=" + query.str();
}
}
