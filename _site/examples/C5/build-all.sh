#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET="esp32c5"

if ! command -v idf.py >/dev/null 2>&1; then
  cat <<'MSG' >&2
idf.py not found. Install or activate ESP-IDF first.

If you use the Espressif installer helpers, a typical flow is:

  idfinstall && idfexport

Then re-run this script from the same shell.
MSG
  exit 1
fi

build_dir() {
  local dir="$1"
  echo "==> Building $(basename "$dir") for ${TARGET}"
  (
    cd "$dir"
    idf.py set-target "${TARGET}" >/dev/null
    idf.py build
  )
}

status=0

# Published C5 workshop track (introduction + assignments 1–6 on the site)
for d in "$SCRIPT_DIR"/c5-workshop/assignment-*; do
  [ -d "$d" ] || continue
  build_dir "$d" || status=1
done

# Optional: same protocols/TLS flow as the ESP32-C6 workshop chapter 6, built for ESP32-C5
extra="$SCRIPT_DIR/c5-extra/c6-ch06-protocols-tls-parity"
if [ -d "$extra" ]; then
  build_dir "$extra" || status=1
fi

exit "$status"
