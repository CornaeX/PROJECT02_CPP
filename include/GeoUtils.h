#ifndef CAMPUS_TOUR_GEO_UTILS_H
#define CAMPUS_TOUR_GEO_UTILS_H

#include "Place.h"
#include <string>

namespace GeoUtils {
    constexpr double EarthRadiusKm = 6371.0088;
    double haversineKm(double latitude1, double longitude1, double latitude2, double longitude2);
    double straightLineDistanceKm(const Place& from, const Place& to);
    std::string googleMapsSearchUrl(const Place& place);
}

#endif
