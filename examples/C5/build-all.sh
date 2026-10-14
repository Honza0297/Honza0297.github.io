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

# Published C5 workshop track (assignments 1–6)
for d in "$SCRIPT_DIR"/assignment-*; do
  [ -d "$d" ] || continue
  build_dir "$d" || status=1
done

exit "$status"
