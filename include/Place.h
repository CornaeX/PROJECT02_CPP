#ifndef CAMPUS_TOUR_PLACE_H
#define CAMPUS_TOUR_PLACE_H

#include <string>

struct Place {
    std::string id;
    std::string name;
    std::string category;
    std::string zone;
    double latitude{};
    double longitude{};
    std::string description;
    std::string sourceUrl;
    std::string verificationStatus;
};

std::string trim(const std::string& value);
std::string toLowerAscii(const std::string& value);
bool isValidCoordinate(double latitude, double longitude);
bool validatePlace(const Place& place, std::string& error);

#endif
