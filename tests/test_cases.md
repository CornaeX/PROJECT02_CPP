# Test Cases

The test executable `campus-tests` performs assertions at runtime and prints each passing test group. Run it with `ctest --test-dir build --output-on-failure`.

| Group | Behavior checked |
|---|---|
| Tree CREATE | Empty state, initialize hierarchy/data, no duplicate reinitialization |
| Tree SEARCH | Search by case-insensitive ID and case-insensitive name substring; missing ID/name |
| Tree INSERT | Add to existing parent, reject duplicate ID, invalid coordinates, missing category |
| Tree DELETE | Delete existing and missing IDs; reject deletion of non-empty category |
| List CREATE | Initially empty and clear/reset behavior |
| List SEARCH | Find by ID and case-insensitive name; report position and next stop; missing result |
| List INSERT | Insert at beginning, middle, and end; invalid position and unknown location ID |
| List DELETE | Delete head, tail, and only node; empty list and out-of-range handling |
| Tree/List consistency | Remove all itinerary references after a location is deleted from the Tree |
| Geography | Haversine known case: one degree of longitude at the equator is about 111.195 km; generated Maps link |
| Input/data validation | Invalid coordinate, missing fields, invalid source URL, malformed data line |
| Startup/UI smoke test | Automatically loads the local TSV at launch; invalid menu values are handled; reload, search, itinerary operations, distance output, and Maps link run through a scripted session |
