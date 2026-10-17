# Actual Test Results

Executed in the Linux container with GNU C++ 14.2.0 and CMake. Results below are from commands run on 17 October 2026.

## Build

Command:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
```

Result: **PASS** — `campus_core`, `campus-tour`, and `campus-tests` compiled and linked successfully with the configured warning flags.

## Automated tests

Command:

```bash
ctest --test-dir build --output-on-failure
./build/campus-tests
```

Result for the final source tree: **58 checks passed, 0 failures**. `ctest` also reported 100% tests passed (1/1 test target).

## Sanitizer tests

Command:

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-asan -j4
ASAN_OPTIONS=detect_leaks=1 ./build-asan/campus-tests
```

Result for the final source tree: **58 checks passed, 0 failures**, with no AddressSanitizer, UndefinedBehaviorSanitizer, or leak report in the captured output.

## Interactive console smoke test

Command:

```bash
./tests/console_smoke.sh
```

The scripted session verified that automatic loading of all 12 local TSV records occurs before the first menu prompt, then entered invalid menu text and an out-of-range option, reloaded the dataset, searched the hospital by ID, created a two-stop itinerary, inserted Student Affairs between existing stops, searched for the next stop, deleted one stop by ID, displayed the updated itinerary, calculated distance, showed a Google Maps URL, and exited.

Result for the updated source tree: **PASS** — the script printed `PASS: console smoke test (invalid menu input, Tree lookup, itinerary insert/search/delete, distance, Maps link).`

A pseudo-terminal (PTY) smoke check also confirmed that an interactive launch shows the auto-load status and sends clear-screen sequences before the menu and after selecting Exit. For redirected/piped input, screen clearing and pauses are intentionally disabled so the smoke-test transcript stays readable.

## Important test-data limitation

Automated coordinate checks verify coordinate range and the Haversine calculation, not on-site survey accuracy. The campus place coordinates are from the individually linked OpenStreetMap-based records; their `VERIFIED_OSM_DERIVED` status does not represent official university certification.

## Console clearing and Thai UTF-8 regression checks

The program was rebuilt after changing the interactive clear behavior and Windows encoding setup. On Windows, `clearConsole()` now tries the native console screen-buffer API first to erase prior buffer text, then falls back to ANSI sequences or `cls`. `configureTerminal()` sets console input/output code pages to UTF-8. CMake passes MSVC `/utf-8` to the app and test targets.

On the Linux build, a scripted `Display all campus locations` session was executed and the output was verified to contain the exact UTF-8 label `NU and CU Book Center (ศูนย์หนังสือ NU และ CU)`. The dataset itself is UTF-8. The Linux Release build, all 58 automated tests, the console smoke test, and the AddressSanitizer/UndefinedBehaviorSanitizer run passed after these changes.

**Platform limitation:** this execution environment is Linux and has no Windows compiler/runtime, so the Windows-specific native screen-buffer API and console code-page branch could not be executed here. Please rebuild on Windows with the documented CMake commands and run the app in Windows Terminal to confirm display behavior on your machine. The MSVC `/utf-8` flag is configured in CMake.
