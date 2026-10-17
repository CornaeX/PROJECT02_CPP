#include "CampusTree.h"
#include "GeoUtils.h"
#include "ItineraryLinkedList.h"
#include "Place.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef CAMPUS_TEST_DATA_PATH
#define CAMPUS_TEST_DATA_PATH "data/places.tsv"
#endif

namespace {
int failures = 0;
int checks = 0;

void check(bool condition, const std::string& description) {
    ++checks;
    if (condition) std::cout << "[PASS] " << description << '\n';
    else {
        ++failures;
        std::cout << "[FAIL] " << description << '\n';
    }
}

Place makePlace(const std::string& id, const std::string& name,
                double lat = 16.75, double lon = 100.19,
                const std::string& category = "Education",
                const std::string& zone = "Central Campus") {
    return Place{id, name, category, zone, lat, lon,
                 "Test location used by automated tests.",
                 "https://example.org/test-source", "TEST_FIXTURE"};
}

std::vector<Place> fixtures() {
    return {
        makePlace("T-001", "Alpha Hall", 16.7500, 100.1900),
        makePlace("T-002", "Beta Library", 16.7500, 101.1900),
        makePlace("T-003", "Gamma Cafe", 16.7510, 100.1910, "Retail and Dining", "Central Campus"),
        makePlace("T-004", "Health Center", 16.7480, 100.1880, "Healthcare", "Medical Campus")
    };
}

void testTree() {
    CampusTree tree;
    check(!tree.initialized(), "new Tree starts empty/uninitialized");
    check(tree.findById("T-001") == nullptr, "searching an empty Tree returns no result");
    std::string error;
    check(tree.deletePlaceById("missing", error) == false, "deleting from uninitialized Tree is safely rejected");
    check(tree.initializeWithPlaces(fixtures(), error), "Tree CREATE initializes hierarchy and fixture locations");
    check(tree.initialized() && tree.allPlaces().size() == 4, "Tree contains initial places after CREATE");
    check(!tree.initializeWithPlaces(fixtures(), error), "duplicate Tree initialization is rejected");
    check(tree.findById("t-001") != nullptr, "Tree SEARCH by case-insensitive location ID works");
    check(tree.searchByName("bEtA").size() == 1, "Tree SEARCH by case-insensitive name substring works");
    check(tree.searchByName("does-not-exist").empty(), "Tree SEARCH reports no match for unknown name");

    check(tree.insertPlace(makePlace("T-005", "New Study Room"), error), "Tree INSERT into existing zone/category works");
    check(!tree.insertPlace(makePlace("t-005", "Duplicate ID"), error), "Tree INSERT rejects duplicate IDs case-insensitively");
    check(!tree.insertPlace(makePlace("T-006", "Bad Coordinate", 91.0, 100.0), error), "Tree INSERT rejects invalid coordinates");
    check(!tree.insertPlace(makePlace("T-007", "Missing Parent", 16.75, 100.19, "Unknown Category", "Central Campus"), error),
          "Tree INSERT rejects nonexistent parent category");
    check(!tree.deleteEmptyCategory("Central Campus", "Education", error),
          "Tree protects a category that still contains locations");
    check(tree.deletePlaceById("T-002", error), "Tree DELETE removes an existing location");
    check(tree.findById("T-002") == nullptr, "Tree DELETE removes location from subsequent searches");
    check(!tree.deletePlaceById("NO-SUCH-ID", error), "Tree DELETE handles missing IDs without crashing");

    CampusTree emptyHierarchyTree;
    check(emptyHierarchyTree.initializeWithPlaces({}, error), "CREATE supports an empty dataset while retaining the hierarchy");
    check(emptyHierarchyTree.deleteEmptyCategory("Sports Campus", "Sports and Recreation", error),
          "Tree allows deletion of an empty category");
    check(!emptyHierarchyTree.deleteEmptyCategory("unknown zone", "unknown category", error),
          "Tree safely rejects deletion from a nonexistent zone/category");
    auto whitespaceId = makePlace(" T-SPACE ", "Whitespace ID");
    check(!emptyHierarchyTree.insertPlace(whitespaceId, error), "Tree INSERT rejects IDs with leading/trailing whitespace");

    CampusTree realTree;
    check(realTree.initializeFromTsv(CAMPUS_TEST_DATA_PATH, error), "real local TSV dataset parses successfully");
    check(realTree.allPlaces().size() >= 8, "real local dataset contains at least eight location records");
    const Place* hospital = realTree.findById("NU-HEALTH-001");
    check(hospital && hospital->sourceUrl.find("https://") == 0, "real dataset preserves a source URL per location");
}

void testLinkedListAndConsistency() {
    CampusTree tree;
    std::string error;
    check(tree.initializeWithPlaces(fixtures(), error), "Tree for itinerary tests initializes");
    ItineraryLinkedList list;
    check(list.empty() && list.size() == 0, "Linked List CREATE starts as an empty itinerary");
    check(!list.deleteAt(1, error), "Linked List DELETE safely handles an empty list");
    check(list.search("T-001", tree).empty(), "Linked List SEARCH on empty list returns no result");
    check(!list.insert("missing-id", 1, tree, error), "Linked List INSERT rejects unknown Tree location IDs");

    check(list.insert("T-001", 1, tree, error), "Linked List INSERT at the beginning works");
    check(list.insert("T-003", 2, tree, error), "Linked List INSERT at the end works");
    check(list.insert("T-002", 2, tree, error), "Linked List INSERT in the middle works");
    check(list.locationIds() == std::vector<std::string>({"T-001", "T-002", "T-003"}),
          "Linked List maintains the requested itinerary order");
    check(!list.insert("T-004", 0, tree, error), "Linked List INSERT rejects position zero");
    check(!list.insert("T-004", 5, tree, error), "Linked List INSERT rejects out-of-range positions");
    const auto result = list.search("beta", tree);
    check(result.size() == 1 && result[0].position == 2 && result[0].nextLocationName == "Gamma Cafe",
          "Linked List SEARCH returns stop position and next stop");
    check(list.search("not-in-itinerary", tree).empty(), "Linked List SEARCH reports absent locations");

    check(list.deleteAt(1, error), "Linked List DELETE removes the head");
    check(list.locationIds() == std::vector<std::string>({"T-002", "T-003"}), "head deletion updates the head link");
    check(list.deleteById("T-003", error), "Linked List DELETE by ID removes the tail stop");
    check(list.locationIds() == std::vector<std::string>({"T-002"}), "tail deletion updates the tail pointer");
    check(list.deleteAt(1, error), "Linked List DELETE removes its only remaining node");
    check(list.empty() && list.size() == 0, "single-node deletion resets head, tail, and size");
    check(!list.deleteAt(1, error), "Linked List DELETE safely handles now-empty list");
    check(!list.deleteById("T-001", error), "Linked List DELETE by absent ID reports failure");

    check(list.insert("T-001", 1, tree, error), "recreate itinerary with a location reference");
    check(list.insert("T-001", 2, tree, error), "itinerary can safely hold repeated stops");
    check(tree.deletePlaceById("T-001", error), "Tree deletion succeeds for referenced location");
    check(list.removeAllById("T-001") == 2, "Tree deletion consistency removes every itinerary reference");
    check(list.empty(), "itinerary is consistent after all references are removed");

    list.create();
    check(list.empty(), "Linked List CREATE clears an existing itinerary");
}

void testGeoAndValidation() {
    const double distance = GeoUtils::haversineKm(0.0, 0.0, 0.0, 1.0);
    check(std::abs(distance - 111.195) < 0.03,
          "Haversine known case: one degree longitude at equator is about 111.195 km");
    bool threw = false;
    try { (void)GeoUtils::haversineKm(100.0, 0.0, 0.0, 0.0); }
    catch (const std::invalid_argument&) { threw = true; }
    check(threw, "geographic utility rejects invalid coordinates");
    const auto place = makePlace("MAP-01", "Map Test");
    check(GeoUtils::googleMapsSearchUrl(place).find("https://www.google.com/maps/search/") == 0,
          "Google Maps URL is generated for a location");

    std::string error;
    auto invalid = makePlace("BAD-URL", "Bad URL");
    invalid.sourceUrl = "not-a-url";
    check(!validatePlace(invalid, error), "validation rejects a malformed source URL");
    invalid = makePlace("BAD-NAME", "   ");
    check(!validatePlace(invalid, error), "validation rejects a missing required name");
}

void testMalformedTsv() {
    const auto temp = std::filesystem::temp_directory_path() / "campus_tour_bad_data.tsv";
    {
        std::ofstream output(temp);
        output << "id\tname\ttoo-few-fields\n";
    }
    CampusTree tree;
    std::string error;
    check(!tree.initializeFromTsv(temp.string(), error), "Tree CREATE rejects malformed TSV input");
    check(!tree.initialized(), "failed TSV initialization leaves Tree uninitialized");
    std::error_code ec;
    std::filesystem::remove(temp, ec);
}
}

int main() {
    std::cout << "Campus Tour Management System - automated tests\n";
    testTree();
    testLinkedListAndConsistency();
    testGeoAndValidation();
    testMalformedTsv();
    std::cout << "\nExecuted " << checks << " checks; failures: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}
