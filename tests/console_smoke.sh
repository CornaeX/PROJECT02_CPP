#!/usr/bin/env sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BIN="$ROOT/build/campus-tour"
LOG="${TMPDIR:-/tmp}/campus-tour-console-smoke.log"

if [ ! -x "$BIN" ]; then
  echo "Missing $BIN; build first with: cmake -S . -B build && cmake --build build"
  exit 2
fi

printf 'abc\n99\n1\n3\nNU-HEALTH-001\n6\n2\nNU-HEALTH-001\nNU-MED-001\n9\nNU-STUDENT-001\n2\n8\nNU-STUDENT-001\n10\ni\nNU-MED-001\n7\n11\n12\nNU-HEALTH-001\n13\n' | "$BIN" > "$LOG"

grep -q 'Invalid input. Enter a menu number' "$LOG"
grep -q 'Menu option must be between 1 and 13' "$LOG"
grep -q 'Campus Tree loaded successfully' "$LOG"
grep -q 'Loaded 12 location(s) from the TSV dataset' "$LOG"
grep -q 'Automatically loaded 12 campus locations' "$LOG"
AUTO_LOAD_LINE=$(grep -n -m1 'Automatically loaded 12 campus locations' "$LOG" | cut -d: -f1)
FIRST_PROMPT_LINE=$(grep -n -m1 'Choose an option (1-13)' "$LOG" | cut -d: -f1)
[ "$AUTO_LOAD_LINE" -lt "$FIRST_PROMPT_LINE" ]
grep -q 'Naresuan University Hospital' "$LOG"
grep -q 'Stop inserted. Current itinerary size: 3' "$LOG"
grep -q 'Next stop: Faculty of Medicine, Naresuan University' "$LOG"
grep -q 'First stop with ID NU-MED-001 deleted' "$LOG"
grep -q 'Total straight-line distance:' "$LOG"
grep -q 'https://www.google.com/maps/search/' "$LOG"
grep -q 'Goodbye.' "$LOG"

echo 'PASS: console smoke test (invalid menu input, Tree lookup, itinerary insert/search/delete, distance, Maps link).'
