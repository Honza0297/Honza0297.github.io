# Workshop Examples

Complete ESP-IDF reference projects for both workshops.

## ESP32-C5 — [examples/C5/](C5/)

| Folder | Assignment |
|---|---|
| `assignment-01-install-verify/` | Assignment 1: verify the ESP-IDF toolchain |
| `assignment-02-led-components/` | Assignment 2: Hello World & LED |
| `assignment-03-wifi-provisioning/` | Assignment 3: Wi-Fi & provisioning |
| `assignment-04-ota/` | Assignment 4: OTA updates (student board) |
| `assignment-04-ota-server/` | Assignment 4: OTA server (second board) |
| `assignment-05-esp-now/` | Assignment 5: ESP-NOW chat room |
| `assignment-06-low-power/` | Assignment 6: LP core |

## ESP32-C6 — [examples/C6/](C6/)

| Folder | Assignment |
|---|---|
| `assignment-01-install-verify/` | Assignment 1: verify the ESP-IDF toolchain |
| `assignment-02-led-components/` | Assignment 2: LED strip |
| `assignment-03-wifi-provisioning/` | Assignment 3: Wi-Fi & provisioning |
| `assignment-04-ota/` | Assignment 4: OTA updates |
| `assignment-05-esp-now/` | Assignment 5: ESP-NOW |
| `assignment-06-low-power/` | Assignment 6: LP core |

On the website, each project folder lists its source files, and the `.zip` next to the folder is that same project.

## Usage

Activate ESP-IDF (e.g. `idfinstall && idfexport`), then from the `C5/` or `C6/` folder run:

```bash
./build-all.sh
```

This builds all projects in sequence and reports any failures.
