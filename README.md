# Naresuan University Smart Campus Navigator

Responsive campus map and itinerary manager built with HTML, CSS, Vanilla JavaScript ES modules, Vite, Leaflet and OpenStreetMap. It reuses stable location IDs and OSM-derived coordinates from the supplied C++ project's `data/places.tsv` and `campus_places_manifest.md`.

## Run

Requires Node.js 18+ and npm.

```bash
npm install
npm run dev
```

Open the URL Vite prints (normally http://localhost:5173). Check tests/build with `npm test` and `npm run build`; production preview is `npm run preview`.

## Features

- Interactive Leaflet map, OSM tiles, zoom/pan, markers, popups and attribution.
- Search by ID/name, category filters, marker focus and location details.
- Real hierarchical Tree supporting CREATE, SEARCH, INSERT and DELETE.
- Custom singly Linked List itinerary supporting CREATE, SEARCH, INSERT at head/middle/tail, DELETE by ID/position, next-stop lookup and ordering controls.
- localStorage persistence and reset-to-seed function. This is local demo storage, not a production database.
- A routing module that can call an OSRM-compatible `/route/v1/foot/` endpoint; when unavailable, it draws dashed straight-line connections and reports only approximate straight-line distance, never walking time.
- New user-entered locations are explicitly unverified.

## Routing

Copy `.env.example` to `.env`, then set `VITE_ROUTING_URL` to a compatible pedestrian-routing service. The service must support the OSRM `/route/v1/foot/` profile; many OSRM public deployments only support driving. If routing fails or is not configured, the app falls back to an approximate straight-line connection. Public demo services have rate limits/usage policies; use an approved endpoint for real use. No API key is committed.

## Structure

- `src/main.js`: app wiring, event handling and synchronization.
- `src/data/places.json`: initial location dataset; TSV and manifest retained for provenance.
- `src/structures/TreeNode.js`, `CampusTree.js`: actual tree nodes and hierarchy/traversal/search/insert/delete.
- `src/structures/ItineraryNode.js`, `ItineraryLinkedList.js`: singly linked itinerary, insertion/deletion/search/head/tail/size.
- `src/services/mapService.js`: Leaflet map, markers, popups and line drawing.
- `src/services/routingService.js`: optional walking provider and approximate fallback.
- `src/services/storageService.js`: localStorage persistence/reset.
- `src/utils/validation.js`, `distance.js`: input validation and haversine distance.
- `tests/`: Node built-in tests for data structures.

## Eight required operations

**Tree:** CREATE initializes root → zone → category → location nodes from JSON/stored data; SEARCH matches exact ID or case-insensitive name; INSERT validates and adds a node under its zone/category; DELETE removes the node and the UI synchronizes markers and itinerary references.

**Linked List:** CREATE resets to an empty itinerary; SEARCH finds a stop and reports its position and next stop; INSERT supports head/tail/arbitrary position and UI ordering; DELETE supports ID or position and maintains head, tail and size. Deleting a location removes all its itinerary references.

## Dataset, attribution and limitations

The 12 initial points come from the supplied manifest and TSV. The manifest describes them as `VERIFIED_OSM_DERIVED` from Mapcarta/OSM-based records at dataset assembly on 17 October 2026; this is not official university certification, so recheck before physical wayfinding. Sources are included in each JSON record.

- OSM copyright and ODbL: https://www.openstreetmap.org/copyright
- Official NU campus map: https://www.nu.ac.th/?page_id=1916
- NU contact/location page: https://www.nu.ac.th/?page_id=5013

Map tiles require internet and attribution. Respect the OSM tile usage policy and do not bulk download tiles. User-created data stays in one browser's localStorage. No geocoding is provided; entered coordinates must be supplied and verified by the user. Browser rendering and network integrations must be checked in a browser with internet access.

## Test status

`npm test` runs Node's built-in tests for tree initialization/traversal/search/insert/delete/validation and linked-list empty/create/search/next-stop/insert/delete/reference cleanup. A successful build checks bundling but does not by itself prove that live map tiles or a third-party routing endpoint work.
