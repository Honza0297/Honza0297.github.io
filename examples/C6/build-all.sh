#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET="esp32c6"

if ! command -v idf.py >/dev/null 2>&1; then
  cat <<'MSG' >&2
idf.py not found. Install or activate ESP-IDF first.

If you use the Espressif installer helpers, a typical flow is:

  idfinstall && idfexport

Then re-run this script from the same shell.
MSG
  exit 1
fi

status=0
for d in "$SCRIPT_DIR"/c6-workshop/assignment-*; do
  [ -d "$d" ] || continue
  echo "==> Building $(basename "$d") for ${TARGET}"
  (
    cd "$d"
    idf.py set-target "${TARGET}" >/dev/null
    idf.py build
  ) || status=1
done

exit "$status"
