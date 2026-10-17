#include "Place.h"

#include <cmath>
#include <cctype>

namespace {
bool containsTsvControl(const std::string& value) {
    return value.find('\t') != std::string::npos ||
           value.find('\n') != std::string::npos ||
           value.find('\r') != std::string::npos;
}
}

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string toLowerAscii(const std::string& value) {
    std::string result = value;
    for (char& ch : result) {
        const unsigned char c = static_cast<unsigned char>(ch);
        ch = static_cast<char>(std::tolower(c));
    }
    return result;
}

bool isValidCoordinate(double latitude, double longitude) {
    return std::isfinite(latitude) && std::isfinite(longitude) &&
           latitude >= -90.0 && latitude <= 90.0 &&
           longitude >= -180.0 && longitude <= 180.0;
}

bool validatePlace(const Place& place, std::string& error) {
    const auto hasControl = [](const std::string& text) {
        return containsTsvControl(text);
    };

    if (trim(place.id).empty()) { error = "Location ID is required."; return false; }
    if (place.id != trim(place.id)) { error = "Location ID cannot have leading or trailing whitespace."; return false; }
    if (trim(place.name).empty()) { error = "Location name is required."; return false; }
    if (trim(place.category).empty()) { error = "Category is required."; return false; }
    if (place.category != trim(place.category)) { error = "Category cannot have leading or trailing whitespace."; return false; }
    if (trim(place.zone).empty()) { error = "Campus zone is required."; return false; }
    if (place.zone != trim(place.zone)) { error = "Campus zone cannot have leading or trailing whitespace."; return false; }
    if (trim(place.description).empty()) { error = "Description is required."; return false; }
    if (trim(place.sourceUrl).empty()) { error = "Source URL is required."; return false; }
    if (!isValidCoordinate(place.latitude, place.longitude)) {
        error = "Coordinates must be finite values (latitude -90..90, longitude -180..180).";
        return false;
    }
    if (place.sourceUrl.rfind("https://", 0) != 0 && place.sourceUrl.rfind("http://", 0) != 0) {
        error = "Source URL must begin with http:// or https://.";
        return false;
    }
    if (hasControl(place.id) || hasControl(place.name) || hasControl(place.category) ||
        hasControl(place.zone) || hasControl(place.description) || hasControl(place.sourceUrl) ||
        hasControl(place.verificationStatus)) {
        error = "Fields cannot contain tabs or line breaks because the dataset is TSV.";
        return false;
    }
    return true;
}
