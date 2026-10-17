#include "CampusTree.h"
#include "GeoUtils.h"
#include "ItineraryLinkedList.h"
#include "Place.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef CAMPUS_DEFAULT_DATA_PATH
#define CAMPUS_DEFAULT_DATA_PATH "data/places.tsv"
#endif

namespace {
const std::string DATA_PATH = CAMPUS_DEFAULT_DATA_PATH;
bool gInteractive = false;
bool gAnsiEnabled = false;

bool terminalIsInteractive() {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) != 0 && _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDIN_FILENO) != 0 && isatty(STDOUT_FILENO) != 0;
#endif
}

void configureTerminal() {
    gInteractive = terminalIsInteractive();
    gAnsiEnabled = gInteractive;
#ifdef _WIN32
    if (gInteractive) {
        // Location names in places.tsv are UTF-8. Tell the Windows console to
        // interpret input/output as UTF-8 instead of the current legacy code page.
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (output == INVALID_HANDLE_VALUE || !GetConsoleMode(output, &mode) ||
            !SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            gAnsiEnabled = false;
        }
    }
#endif
}

std::string paint(const std::string& text, const char* code) {
    if (!gAnsiEnabled) return text;
    return std::string(code) + text + "\033[0m";
}

void clearConsole() {
    // Keep output readable when redirected to a log or piped by the test suite.
    if (!gInteractive) return;
#ifdef _WIN32
    // Use the native console API first. CSI 2J often clears only the visible
    // viewport (leaving old screens in scrollback), which makes the UI appear
    // to append text instead of replacing the previous screen.
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (output != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(output, &info)) {
        const DWORD cells = static_cast<DWORD>(info.dwSize.X) *
                            static_cast<DWORD>(info.dwSize.Y);
        const COORD origin{0, 0};
        DWORD written = 0;
        const BOOL charsCleared = FillConsoleOutputCharacterA(
            output, ' ', cells, origin, &written);
        const BOOL attrsCleared = FillConsoleOutputAttribute(
            output, info.wAttributes, cells, origin, &written);
        if (charsCleared && attrsCleared && SetConsoleCursorPosition(output, origin)) {
            std::cout.flush();
            return;
        }
    }
    // Fallback for terminals that do not expose a classic Windows screen buffer.
    if (gAnsiEnabled) std::cout << "\033[3J\033[2J\033[H";
    else std::system("cls");
#else
    // CSI 3J also requests clearing the terminal scrollback where supported.
    std::cout << "\033[3J\033[2J\033[H";
#endif
    std::cout.flush();
}

void pauseForEnter() {
    // In interactive use, let the user read the result before the next screen replaces it.
    // In non-interactive use, don't consume the next command from a pipe or smoke test.
    if (!gInteractive) return;
    std::cout << "\n" << paint("Press Enter to return to the menu...", "\033[90m");
    std::string ignored;
    std::getline(std::cin, ignored);
}

bool askLine(const std::string& prompt, std::string& value) {
    std::cout << paint(prompt, "\033[96m");
    if (!std::getline(std::cin, value)) return false;
    value = trim(value);
    return true;
}

bool parseInteger(const std::string& text, int& value) {
    try {
        std::size_t used = 0;
        const int parsed = std::stoi(text, &used);
        if (used != text.size()) return false;
        value = parsed;
        return true;
    } catch (...) { return false; }
}

bool parseSize(const std::string& text, std::size_t& value) {
    try {
        if (text.empty() || text.front() == '-') return false;
        std::size_t used = 0;
        const unsigned long long parsed = std::stoull(text, &used);
        if (used != text.size() || parsed > 1000000ULL) return false;
        value = static_cast<std::size_t>(parsed);
        return true;
    } catch (...) { return false; }
}

bool parseDouble(const std::string& text, double& value) {
    try {
        std::size_t used = 0;
        value = std::stod(text, &used);
        return used == text.size() && std::isfinite(value);
    } catch (...) { return false; }
}

bool askSize(const std::string& prompt, std::size_t& value) {
    std::string text;
    if (!askLine(prompt, text)) return false;
    if (!parseSize(text, value)) {
        std::cout << paint("Invalid input: please enter a non-negative whole number.\n", "\033[31m");
        value = static_cast<std::size_t>(-1);
    }
    return true;
}

void printRule() {
    std::cout << paint("+------------------------------------------------------------------------------+\n", "\033[90m");
}

void printBanner() {
    printRule();
    std::cout << paint("|                    CAMPUS TOUR MANAGEMENT SYSTEM                            |\n", "\033[96;1m")
              << paint("|                              C++17 CONSOLE                                  |\n", "\033[96;1m");
    printRule();
}

void printMainMenu(const CampusTree& tree, const ItineraryLinkedList& itinerary,
                   const std::string& startupMessage) {
    printBanner();
    const std::string treeState = tree.initialized()
        ? paint("READY", "\033[32;1m") : paint("NOT LOADED", "\033[31;1m");
    std::cout << "  Dataset : " << DATA_PATH << "\n"
              << "  Tree    : " << treeState;
    if (tree.initialized()) std::cout << "  |  Locations: " << tree.allPlaces().size();
    std::cout << "\n  Itinerary: " << itinerary.size() << " stop(s)\n";
    if (!startupMessage.empty()) {
        std::cout << "  Status  : " << startupMessage << "\n";
    }
    printRule();
    std::cout << paint("  CAMPUS TREE / LOCATION DATABASE\n", "\033[94;1m")
              << "   1. Reload campus data from TSV file\n"
              << "   2. Display all campus locations\n"
              << "   3. Search campus location\n"
              << "   4. Insert campus location\n"
              << "   5. Delete campus location\n\n"
              << paint("  ITINERARY / SINGLY LINKED LIST\n", "\033[94;1m")
              << "   6. Create a new itinerary\n"
              << "   7. Display itinerary\n"
              << "   8. Search itinerary stop\n"
              << "   9. Insert itinerary stop\n"
              << "  10. Delete itinerary stop\n"
              << "  11. Calculate itinerary distances\n\n"
              << paint("  MAP AND EXIT\n", "\033[94;1m")
              << "  12. Display Google Maps link\n"
              << "  13. Exit\n";
    printRule();
    std::cout << paint("  Tip: enter a menu number; this screen refreshes after every action.\n", "\033[90m");
}

const char* actionTitle(int choice) {
    switch (choice) {
        case 1: return "RELOAD CAMPUS DATA";
        case 2: return "CAMPUS TREE — ALL LOCATIONS";
        case 3: return "SEARCH CAMPUS LOCATION";
        case 4: return "INSERT CAMPUS LOCATION";
        case 5: return "DELETE CAMPUS LOCATION";
        case 6: return "CREATE NEW ITINERARY";
        case 7: return "ITINERARY — LINKED LIST";
        case 8: return "SEARCH ITINERARY STOP";
        case 9: return "INSERT ITINERARY STOP";
        case 10: return "DELETE ITINERARY STOP";
        case 11: return "ITINERARY DISTANCES";
        case 12: return "GOOGLE MAPS LINK";
        default: return "CAMPUS TOUR MANAGEMENT SYSTEM";
    }
}

void printActionHeader(int choice) {
    printBanner();
    std::cout << "  > " << paint(actionTitle(choice), "\033[93;1m") << "\n";
    printRule();
}

void printPlace(const Place& place) {
    std::cout << "\n" << paint(place.name, "\033[96;1m") << "\n"
              << "  ID          : " << place.id
              << "\n  Category    : " << place.category
              << "\n  Campus zone : " << place.zone
              << "\n  Coordinates : " << std::fixed << std::setprecision(6)
              << place.latitude << ", " << place.longitude
              << "\n  Description : " << place.description
              << "\n  Data status : " << place.verificationStatus
              << "\n  Source      : " << place.sourceUrl
              << "\n  Google Maps : " << GeoUtils::googleMapsSearchUrl(place) << "\n";
}

const Place* findPlaceByQuery(const CampusTree& tree, const std::string& query,
                              std::vector<Place>& nameMatches) {
    if (const Place* byId = tree.findById(query)) return byId;
    nameMatches = tree.searchByName(query);
    if (nameMatches.size() == 1) return tree.findById(nameMatches.front().id);
    return nullptr;
}

void searchCampus(const CampusTree& tree) {
    std::string query;
    if (!askLine("Enter location ID or part of its name: ", query)) return;
    if (query.empty()) { std::cout << "Search text cannot be empty.\n"; return; }
    if (const Place* place = tree.findById(query)) {
        printPlace(*place);
        return;
    }
    const auto matches = tree.searchByName(query);
    if (matches.empty()) {
        std::cout << "No campus location matched '" << query << "'.\n";
        return;
    }
    std::cout << paint("Found " + std::to_string(matches.size()) + " location(s):\n", "\033[32m");
    for (const auto& place : matches) printPlace(place);
}

bool getPlaceFromUser(Place& place) {
    std::string latitudeText, longitudeText;
    if (!askLine("New unique location ID: ", place.id) ||
        !askLine("Location name: ", place.name) ||
        !askLine("Exact category (see Tree display): ", place.category) ||
        !askLine("Exact campus zone (see Tree display): ", place.zone) ||
        !askLine("Latitude (-90..90): ", latitudeText) ||
        !askLine("Longitude (-180..180): ", longitudeText) ||
        !askLine("Short description: ", place.description) ||
        !askLine("Source URL (http/https): ", place.sourceUrl)) return false;
    if (!parseDouble(latitudeText, place.latitude) || !parseDouble(longitudeText, place.longitude)) {
        std::cout << paint("Invalid coordinates: use numeric latitude and longitude.\n", "\033[31m");
        return true;
    }
    place.verificationStatus = "UNVERIFIED_USER_ADDED";
    return true;
}

void saveAfterMutation(const CampusTree& tree) {
    std::string error;
    if (!tree.saveToTsv(DATA_PATH, error)) {
        std::cout << paint("Warning: change succeeded in memory but could not be saved: ", "\033[31m")
                  << error << "\n";
    } else {
        std::cout << paint("Saved changes to dataset: ", "\033[32m") << DATA_PATH << "\n";
    }
}

void reloadCampusTree(CampusTree& tree, ItineraryLinkedList& itinerary) {
    CampusTree reloaded;
    std::string error;
    if (!reloaded.initializeFromTsv(DATA_PATH, error)) {
        std::cout << paint("Reload failed: ", "\033[31m") << error << "\n";
        return;
    }
    std::size_t removed = 0;
    for (const auto& id : itinerary.locationIds()) {
        if (!reloaded.findById(id)) removed += itinerary.removeAllById(id);
    }
    tree = std::move(reloaded);
    std::cout << paint("Campus Tree loaded successfully.\n", "\033[32;1m")
              << "Loaded " << tree.allPlaces().size() << " location(s) from the TSV dataset.\n";
    if (removed > 0) {
        std::cout << "Removed " << removed << " itinerary stop(s) whose IDs no longer exist in the dataset.\n";
    }
}

void insertCampusLocation(CampusTree& tree) {
    Place place;
    if (!getPlaceFromUser(place)) return;
    if (!isValidCoordinate(place.latitude, place.longitude)) {
        std::cout << paint("Coordinates are outside valid latitude/longitude ranges. Insert cancelled.\n", "\033[31m");
        return;
    }
    std::string error;
    if (!tree.insertPlace(place, error)) {
        std::cout << paint("Insert failed: ", "\033[31m") << error << "\n";
        return;
    }
    std::cout << paint("Location inserted into the Tree.", "\033[32;1m")
              << " It is marked UNVERIFIED until reviewed.\n";
    saveAfterMutation(tree);
}

void deleteCampusLocation(CampusTree& tree, ItineraryLinkedList& itinerary) {
    std::string id;
    if (!askLine("Enter location ID to delete: ", id)) return;
    std::string error;
    if (!tree.deletePlaceById(id, error)) {
        std::cout << paint("Delete failed: ", "\033[31m") << error << "\n";
        return;
    }
    const std::size_t removedStops = itinerary.removeAllById(id);
    std::cout << paint("Location deleted from the Tree. ", "\033[32;1m")
              << "Removed " << removedStops << " itinerary stop(s) that referenced this ID.\n";
    saveAfterMutation(tree);
}

void createItinerary(ItineraryLinkedList& itinerary, const CampusTree& tree) {
    itinerary.create();
    std::cout << paint("Created a new empty itinerary.\n", "\033[32m");
    std::size_t requested = 0;
    if (!askSize("How many initial stops would you like to add? ", requested)) return;
    if (requested == static_cast<std::size_t>(-1) || requested > 1000) {
        std::cout << paint("Please choose between 0 and 1000 initial stops.\n", "\033[31m");
        return;
    }
    for (std::size_t i = 0; i < requested; ++i) {
        std::string id;
        if (!askLine("Location ID for stop " + std::to_string(i + 1) + ": ", id)) return;
        std::string error;
        if (!itinerary.insert(id, itinerary.size() + 1, tree, error)) {
            std::cout << paint("Stop not added: ", "\033[31m") << error << "\n";
        } else {
            std::cout << paint("Added as stop " + std::to_string(itinerary.size()) + ".\n", "\033[32m");
        }
    }
}

void searchItinerary(const ItineraryLinkedList& itinerary, const CampusTree& tree) {
    std::string query;
    if (!askLine("Enter location ID or part of its name: ", query)) return;
    const auto matches = itinerary.search(query, tree);
    if (matches.empty()) {
        std::cout << paint("No matching stop was found in the itinerary.\n", "\033[33m");
        return;
    }
    for (const auto& match : matches) {
        std::cout << paint("Stop " + std::to_string(match.position) + ": ", "\033[96;1m")
                  << match.locationName << " [" << match.locationId << "]\n  Next stop: "
                  << match.nextLocationName << "\n";
    }
}

void insertItineraryStop(ItineraryLinkedList& itinerary, const CampusTree& tree) {
    std::string id;
    if (!askLine("Location ID to insert: ", id)) return;
    std::size_t position = 0;
    if (!askSize("Insert at position (1 = beginning, size+1 = end): ", position)) return;
    std::string error;
    if (!itinerary.insert(id, position, tree, error)) {
        std::cout << paint("Insert failed: ", "\033[31m") << error << "\n";
        return;
    }
    std::cout << paint("Stop inserted. ", "\033[32;1m")
              << "Current itinerary size: " << itinerary.size() << ".\n";
}

void deleteItineraryStop(ItineraryLinkedList& itinerary) {
    std::string mode;
    if (!askLine("Delete by [I]D or [P]osition? ", mode)) return;
    std::string error;
    if (toLowerAscii(mode) == "i" || toLowerAscii(mode) == "id") {
        std::string id;
        if (!askLine("Location ID: ", id)) return;
        if (!itinerary.deleteById(id, error)) std::cout << paint("Delete failed: ", "\033[31m") << error << "\n";
        else std::cout << paint("First stop with ID " + id + " deleted.\n", "\033[32m");
    } else if (toLowerAscii(mode) == "p" || toLowerAscii(mode) == "position") {
        std::size_t position = 0;
        if (!askSize("One-based stop position: ", position)) return;
        if (position == static_cast<std::size_t>(-1) || !itinerary.deleteAt(position, error))
            std::cout << paint("Delete failed: ", "\033[31m") << (error.empty() ? "Invalid position." : error) << "\n";
        else std::cout << paint("Stop deleted.\n", "\033[32m");
    } else {
        std::cout << paint("Choose I/ID or P/Position.\n", "\033[31m");
    }
}

void displayDistances(const ItineraryLinkedList& itinerary, const CampusTree& tree) {
    const auto ids = itinerary.locationIds();
    if (ids.empty()) {
        std::cout << "Itinerary is empty. Total straight-line distance: 0.000 km\n";
        return;
    }
    double totalKm = 0.0;
    std::cout << paint("STRAIGHT-LINE DISTANCES (not walking routes)\n\n", "\033[96;1m");
    for (std::size_t i = 0; i < ids.size(); ++i) {
        const Place* current = tree.findById(ids[i]);
        std::cout << std::setw(2) << (i + 1) << ". "
                  << (current ? current->name : "[MISSING: " + ids[i] + "]");
        if (i > 0) {
            const Place* previous = tree.findById(ids[i - 1]);
            if (previous && current) {
                const double distance = GeoUtils::straightLineDistanceKm(*previous, *current);
                totalKm += distance;
                std::cout << "\n    + " << std::fixed << std::setprecision(3) << distance << " km from previous stop";
            } else {
                std::cout << "\n    + distance unavailable (missing location data)";
            }
        } else {
            std::cout << "\n    Start of itinerary";
        }
        std::cout << "\n\n";
    }
    printRule();
    std::cout << "  Total straight-line distance: " << paint([&] {
        std::ostringstream result;
        result << std::fixed << std::setprecision(3) << totalKm << " km";
        return result.str();
    }(), "\033[92;1m") << "\n";
}

void displayMapsLink(const CampusTree& tree) {
    std::string query;
    if (!askLine("Enter location ID or name: ", query)) return;
    std::vector<Place> matches;
    if (const Place* place = findPlaceByQuery(tree, query, matches)) {
        std::cout << paint(place->name, "\033[96;1m") << "\n" << GeoUtils::googleMapsSearchUrl(*place) << "\n";
        return;
    }
    if (matches.empty()) {
        std::cout << paint("No matching location found.\n", "\033[33m");
    } else {
        std::cout << "Multiple matches; choose a more specific name or use an ID:\n";
        for (const auto& place : matches) {
            std::cout << place.id << " | " << place.name << "\n  "
                      << GeoUtils::googleMapsSearchUrl(place) << '\n';
        }
    }
}
}

int main() {
    configureTerminal();
    CampusTree tree;
    ItineraryLinkedList itinerary;
    std::string startupMessage;

    // Automatically load local records at program launch, so users can use the app immediately.
    std::string loadError;
    if (tree.initializeFromTsv(DATA_PATH, loadError)) {
        startupMessage = "Automatically loaded " + std::to_string(tree.allPlaces().size()) + " campus locations.";
    } else {
        startupMessage = "Could not auto-load data: " + loadError + ". Use option 1 to retry.";
    }

    while (true) {
        clearConsole();
        printMainMenu(tree, itinerary, startupMessage);
        std::string choiceText;
        if (!askLine("\nChoose an option (1-13): ", choiceText)) break;

        int choice = -1;
        if (!parseInteger(choiceText, choice)) {
            clearConsole();
            printBanner();
            std::cout << paint("Invalid input. Enter a menu number from 1 to 13.\n", "\033[31;1m");
            pauseForEnter();
            continue;
        }
        if (choice < 1 || choice > 13) {
            clearConsole();
            printBanner();
            std::cout << paint("Menu option must be between 1 and 13.\n", "\033[31;1m");
            pauseForEnter();
            continue;
        }
        if (choice == 13) {
            clearConsole();
            printBanner();
            std::cout << paint("Goodbye! Thanks for using Campus Tour Management System.\n", "\033[92;1m");
            break;
        }

        // Replace the menu screen with a clean action screen after a selection is made.
        clearConsole();
        printActionHeader(choice);
        if (choice == 1) {
            reloadCampusTree(tree, itinerary);
            startupMessage = "Dataset loaded from the local TSV file.";
        } else if (!tree.initialized()) {
            std::cout << paint("Campus data is not loaded. Use option 1 to load data/places.tsv.\n", "\033[31;1m");
        } else {
            switch (choice) {
                case 2: tree.displayHierarchy(std::cout); break;
                case 3: searchCampus(tree); break;
                case 4: insertCampusLocation(tree); break;
                case 5: deleteCampusLocation(tree, itinerary); break;
                case 6: createItinerary(itinerary, tree); break;
                case 7: itinerary.display(tree, std::cout); break;
                case 8: searchItinerary(itinerary, tree); break;
                case 9: insertItineraryStop(itinerary, tree); break;
                case 10: deleteItineraryStop(itinerary); break;
                case 11: displayDistances(itinerary, tree); break;
                case 12: displayMapsLink(tree); break;
                default: break;
            }
        }
        pauseForEnter();
    }
    return 0;
}
