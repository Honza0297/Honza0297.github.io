# Workshop Examples

Complete ESP-IDF reference projects for both workshops.

## ESP32-C5 — [examples/C5/](C5/)

| Folder | Assignment |
|---|---|
| `assignment-01-install-verify/` | Assignment 1: Hello World / verify toolchain |
| `assignment-02-led-components/` | Assignment 2: LED strip with led_strip component |
| `assignment-03-wifi-station/` | Assignment 3: Wi-Fi station mode |
| `assignment-04-nvs/` | Assignment 4: NVS credentials storage |
| `assignment-05-wifi-provisioning/` | Assignment 5: BLE Wi-Fi provisioning |
| `assignment-06-low-power/` | Assignment 6: ULP / LP core |
| `extra-protocols-tls/` | Extra: HTTPS/TLS parity example (C6 ch06 equivalent) |

## ESP32-C6 — [examples/C6/](C6/)

| Folder | Assignment |
|---|---|
| `assignment-01-install-verify/` | Assignment 1: Hello World / verify toolchain |
| `assignment-02-led-components/` | Assignment 2: LED strip with BSP |
| `assignment-03-wifi-station/` | Assignment 3: Wi-Fi station mode |
| `assignment-04-nvs/` | Assignment 4: NVS credentials storage |
| `assignment-05-wifi-provisioning/` | Assignment 5: BLE Wi-Fi provisioning |
| `assignment-06-protocols-tls/` | Assignment 6: HTTPS / TLS with certificate bundle |
| `assignment-07-low-power/` | Assignment 7: ULP / LP core |

## Usage

Activate ESP-IDF (e.g. `idfinstall && idfexport`), then from the `C5/` or `C6/` folder run:

```bash
./build-all.sh
```

This builds all projects in sequence and reports any failures.
