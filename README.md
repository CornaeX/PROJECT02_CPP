# Campus Tour Management System — C++ Console Edition

A C++17 terminal application for organizing Naresuan University campus location records and building an ordered multi-stop itinerary. It is built for a Data Structures assignment and uses a real custom hierarchical Tree and a custom Singly Linked List.

## Features

- Hierarchical location database: university root → campus zone → category → location.
- Tree CREATE, SEARCH by ID/name, INSERT under an existing category, and DELETE by location ID.
- A custom singly linked list storing location IDs in visit order, with CREATE, SEARCH, INSERT at a 1-based position, and DELETE by ID or position.
- Stable IDs link the data structures. Deleting a location from the Tree removes all occurrences of its ID from the active itinerary.
- Latitude/longitude validation, per-record source URL, verification-status field, TSV data file, and a manifest for keeping a future website edition consistent.
- Haversine straight-line distances between consecutive itinerary stops and a total straight-line distance.
- Google Maps search URLs; this console program does not render a live map or generate pedestrian directions.
- CMake build and an automated test executable.

## Why these data structures?

### Tree for the campus database

A campus is naturally hierarchical: zones contain categories, and categories contain locations. The implementation uses a custom `TreeNode` with an explicit node type and owned child nodes. Searching, listing, inserting, and deleting traverse or mutate that hierarchy directly; the Tree is not replaced by a flat lookup table. `std::vector<std::unique_ptr<TreeNode>>` stores each node's children, while each node's parent/category is represented by its position in the custom hierarchy.

`CREATE` builds a fixed campus hierarchy and loads the TSV records, while refusing a second initialization on the same object. `SEARCH` recursively traverses the Tree by case-insensitive ID or name. `INSERT` requires a unique ID, valid fields/coordinates, a URL, and a pre-existing zone/category. `DELETE` removes the matching location node through its owning `unique_ptr`, so memory is released safely. The internal API refuses to delete a category while it still has children; the console menu intentionally only offers location deletion.

### Singly Linked List for the itinerary

An itinerary is ordered and often changes by adding/removing stops. A custom singly linked list makes the order visible as `head → node → node → null`; each node stores only a stable location ID, not a pointer to a Tree node. The list has a head pointer, a non-owning tail pointer, and a size counter. Insertions/deletions update these carefully, including empty/single-node/head/tail cases. Searching resolves IDs through the Tree to show names and the next stop.

The list may include the same location more than once. If an underlying location is deleted, the application removes all itinerary nodes referring to that ID. The list does not retain a potentially dangling pointer to a Tree node.

## Folder layout

```text
campus-tour-cpp/
├── CMakeLists.txt
├── README.md
├── data/
│   ├── places.tsv
│   └── campus_places_manifest.md
├── include/
│   ├── Place.h
│   ├── CampusTree.h
│   ├── ItineraryLinkedList.h
│   └── GeoUtils.h
├── src/
│   ├── main.cpp
│   ├── Place.cpp
│   ├── CampusTree.cpp
│   ├── ItineraryLinkedList.cpp
│   └── GeoUtils.cpp
└── tests/
    ├── test_main.cpp
    └── test_cases.md
```

## Build and run

### Linux (GCC/Clang, CMake)

From the project directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/campus-tour
```

Run automated tests:

```bash
ctest --test-dir build --output-on-failure
# Or run the test binary directly:
./build/campus-tests
```

### Windows (Visual Studio Build Tools or Visual Studio with CMake)

Open a Developer PowerShell/Command Prompt in the project directory, then:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\campus-tour.exe
ctest --test-dir build -C Release --output-on-failure
```

If your CMake generator is single-configuration (for example, MinGW Makefiles), the executable may instead be `build\campus-tour.exe` and the test binary `build\campus-tests.exe`.

### Direct compiler command (Linux)

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude \
  src/main.cpp src/Place.cpp src/CampusTree.cpp \
  src/ItineraryLinkedList.cpp src/GeoUtils.cpp \
  -o campus-tour
./campus-tour
```

CMake supplies an absolute default path to `data/places.tsv`, so the packaged executable can locate the local dataset even if you launch it from a different current directory in the same source tree. The source data file must remain accessible and writable for automatic saves.

## Basic use / example workflow

1. Start the program. It automatically loads `data/places.tsv` before showing the menu.
2. Choose `2` to inspect the actual zone/category/location hierarchy.
3. Choose `3` and search for `Naresuan University Hospital` or `NU-HEALTH-001`.
4. Choose `6` to create an empty itinerary and add two or more location IDs.
5. Choose `9` to insert another location at position `2`, between two existing stops.
6. Choose `8` and search a stop by name to see its position and next stop.
7. Choose `10` to delete a stop, then `7` to inspect the updated linked list.
8. Choose `11` to print straight-line segment distances and the total. Choose `12` to display a Google Maps URL.

When running in an interactive terminal, the interface clears the previous screen after each menu selection, displays the selected action on a fresh screen, and waits for Enter before returning to the menu. On Windows, it uses the native screen-buffer API first so old text is cleared rather than left in the console buffer; it falls back to ANSI clear sequences or `cls` if needed. On other terminals, it also requests scrollback clearing where supported. When input/output is redirected, clearing and pausing are disabled so commands can be scripted and logs remain readable. Option `1` reloads the current local TSV file if you want to refresh the in-memory Tree.

The TSV dataset is UTF-8. On Windows, the program selects UTF-8 console input/output code pages and CMake adds MSVC's `/utf-8` compiler option. Use Windows Terminal or another terminal with a font supporting Thai characters if you want Thai names to display correctly. Do not re-save `data/places.tsv` as ANSI or a legacy code page.

Adding/deleting campus locations through the program saves the updated Tree's location records to `data/places.tsv`. New user-entered records are marked `UNVERIFIED_USER_ADDED`. Check name, source and coordinates before treating them as reliable. The active itinerary is in memory and resets when the program exits.

## Console menu

1. Reload Campus Dataset (data loads automatically at startup)
2. Display All Campus Locations
3. Search Campus Location
4. Insert Campus Location
5. Delete Campus Location
6. Create New Itinerary
7. Display Itinerary
8. Search Itinerary Stop
9. Insert Itinerary Stop
10. Delete Itinerary Stop
11. Calculate Itinerary Distances
12. Display Google Maps Link
13. Exit

Input is read as whole lines and parsed before use. Nonnumeric menu/position inputs, nonexistent IDs, invalid coordinates, duplicate IDs, and invalid list positions report errors rather than being treated as valid input.

## Location data and limitations

The starter TSV includes 12 campus place records. Coordinates are taken from the source pages linked on each row; those Mapcarta pages identify the mapped features as based on OpenStreetMap and related data. The `VERIFIED_OSM_DERIVED` status means the coordinate and map feature were found in the linked mapping record during dataset assembly. It is **not** a claim of official university certification or an on-the-ground survey. Review `data/campus_places_manifest.md` before extending or reusing the dataset.

Sources:

- [Official Naresuan University campus map](https://www.nu.ac.th/?page_id=1916)
- [Official Naresuan University location/contact page](https://www.nu.ac.th/?page_id=5013)
- [OpenStreetMap copyright and attribution](https://www.openstreetmap.org/copyright)
- OSM-based point records linked individually in `data/places.tsv` and `data/campus_places_manifest.md`

OpenStreetMap data has an Open Database License (ODbL) attribution requirement when redistributed; maintain attribution and verify the current license conditions. This is a locally loaded starter dataset: location updates from the internet are not fetched by the application.

The Haversine calculation estimates the shortest arc along a spherical Earth between two coordinate points. The itinerary total is the sum of the consecutive straight-line segments—not walking distance, road distance, travel time, accessibility guidance, or turn-by-turn directions. The displayed Google Maps URL opens the selected coordinate in a real map service but does not itself calculate a route.

## Data extension and stable IDs

Use `data/campus_places_manifest.md` as the stable record contract shared with a later website edition. Add a new tab-separated line containing exactly nine fields: `id`, `name`, `category`, `zone`, `latitude`, `longitude`, `description`, `source_url`, `verification_status`. Keep the source URL and verification status honest. The zone and category must already be present in the hierarchy built by `CampusTree::makeDefaultHierarchy()`; to add a category, update that hierarchy and both the documentation and dataset together. Never recycle an old location ID for a different place.

## Automated tests

The `campus-tests` executable currently passes 58 checks covering Tree CREATE/SEARCH/INSERT/DELETE, Linked List CREATE/SEARCH/INSERT/DELETE, edge cases, duplicate IDs, empty and malformed data, invalid fields/coordinates, category-parent validation, repeated stops, Tree-to-itinerary consistency, Google Maps URL formatting, and a known Haversine distance. Run `./tests/console_smoke.sh` on Linux to smoke-test the menu and an end-to-end sample interaction. See `tests/TEST_RESULTS.md` for actual build/test results and `tests/test_cases.md` for the case inventory.

## Short demonstration script (virtual skit / role-play)

**Roles:** Student visitor and Campus Tour Assistant.

**Student:** “I am visiting NU for the first time. Where is the university hospital?”

**Assistant:** “I will search the campus Tree.” *(Select menu 3, enter `Naresuan University Hospital`; show the name, category, coordinates, source and Maps URL.)*

**Student:** “Please start an itinerary with the hospital and the Faculty of Medicine.” *(Select menu 6 and enter `NU-HEALTH-001` and `NU-MED-001`.)*

**Student:** “Can we visit the Student Affairs Division between those two stops?” *(Select menu 9, enter `NU-STUDENT-001`, position `2`.)*

**Assistant:** “The linked list now places Student Affairs at stop 2. I’ll search it so we can see what comes next.” *(Select menu 8, enter `NU-STUDENT-001`; show position and next stop.)*

**Student:** “Remove the Faculty of Medicine from this itinerary.” *(Select menu 10, choose ID, enter `NU-MED-001`.)*

**Assistant:** “Done. Let’s inspect the updated route and its straight-line estimates.” *(Select menu 7 to show the list, then menu 11 for consecutive distances. Remind the audience these are not walking directions.)*

**Student:** “Finally, show me the map for the hospital.” *(Select menu 12 and enter `NU-HEALTH-001`; show the Google Maps URL.)*
