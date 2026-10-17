# Campus Places Manifest

This manifest is the shared ID reference for the C++ console edition and a future website edition. Keep IDs stable once published; if a place is renamed, retain its ID. If a place is removed permanently, do not recycle its ID for a different place.

## Coordinate and verification policy

- Coordinates are latitude/longitude in decimal degrees.
- `VERIFIED_OSM_DERIVED` means the coordinate and mapped feature were visible on the cited OpenStreetMap-based Mapcarta record when this starter dataset was assembled (17 October 2026). It does **not** mean the university has officially certified the coordinate or that the place name is guaranteed to be current.
- The source URL for each record is stored directly in `places.tsv` and should be rechecked before using these data for physical wayfinding.
- Distances calculated by this project are straight-line distances. They are not walking distances or pedestrian directions.

## Stable locations

| Stable ID | Name | Category | Zone | Latitude | Longitude | Source |
|---|---|---|---|---:|---:|---|
| `NU-HEALTH-001` | Naresuan University Hospital | Healthcare | Medical Campus | 16.74855 | 100.18936 | https://mapcarta.com/N6384784351 |
| `NU-MED-001` | Faculty of Medicine, Naresuan University | Medical Education | Medical Campus | 16.74752 | 100.18855 | https://mapcarta.com/W665609208 |
| `NU-SPORT-001` | Indoor Sports Building | Sports and Recreation | Sports Campus | 16.74711 | 100.19735 | https://mapcarta.com/W131380541 |
| `NU-SPORT-002` | Outdoor Athletics Track | Sports and Recreation | Sports Campus | 16.75119 | 100.19389 | https://mapcarta.com/W131383775 |
| `NU-STUDENT-001` | Student Affairs Division | Student Services | Central Campus | 16.75101 | 100.19284 | https://mapcarta.com/W182692793 |
| `NU-SHOP-001` | NU and CU Book Center (ศูนย์หนังสือ NU และ CU) | Retail and Dining | Central Campus | 16.74518 | 100.19349 | https://mapcarta.com/N7874928023 |
| `NU-SERVICE-001` | IT Station | University Services | Central Campus | 16.74693 | 100.19828 | https://mapcarta.com/W178645624 |
| `NU-SERVICE-002` | Project Administration Office | University Services | Central Campus | 16.75168 | 100.19024 | https://mapcarta.com/W180245070 |
| `NU-SERVICE-003` | Saengphrom Land Office | University Services | Central Campus | 16.75430 | 100.19419 | https://mapcarta.com/W180246894 |
| `NU-DINING-001` | Muang Thong Market Project | Retail and Dining | Central Campus | 16.75279 | 100.19224 | https://mapcarta.com/W180245067 |
| `NU-SERVICE-004` | Thep Therat Hall (หอพระเทพรัตน์) | University Services | Medical Campus | 16.74883 | 100.19080 | https://mapcarta.com/W1125088050 |
| `NU-BANK-001` | Krung Thai Bank, Naresuan University | Retail and Dining | Central Campus | 16.73743 | 100.19921 | https://mapcarta.com/N1908842082 |

## Updating the dataset

1. Confirm the name and point location on OpenStreetMap and, where possible, an official Naresuan University source.
2. Assign a new stable ID; do not derive IDs from row position.
3. Add one tab-separated row to `places.tsv` using the exact `zone` and `category` hierarchy supported by the app.
4. Set verification status honestly. Use `UNVERIFIED` for a record whose location could not be independently checked.
5. Update this manifest in the same change so the future website uses the same IDs and coordinates.

## Attribution and license note

Location geometry/name information is based on OpenStreetMap data accessed through Mapcarta records. OpenStreetMap data is available under the Open Database License (ODbL). Attribute OpenStreetMap contributors when redistributing derived geographic data, and review the current OSM attribution and license requirements at https://www.openstreetmap.org/copyright and https://www.openstreetmap.org. The university's official campus-map page is https://www.nu.ac.th/?page_id=1916 and its official contact/location page is https://www.nu.ac.th/?page_id=5013.
