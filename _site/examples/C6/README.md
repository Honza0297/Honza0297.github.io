# ESP-IDF workshop examples — ESP32-C6

Reference ESP-IDF projects aligned with the [ESP32-C6 workshop](../../content/workshops/esp-idf-with-esp32-c6/_index.md): introduction + assignments **1–7**.

## Environment

You need `idf.py` on your `PATH`. With the Espressif installer helpers, a typical sequence is:

```bash
idfinstall && idfexport
```

Then:

```bash
./build-all.sh
```

This sets the target to **esp32c6** and builds every project under `c6-workshop/`.

## Layout

| Workshop chapter | Directory |
| --- | --- |
| (env check) | `c6-workshop/assignment-01-install-verify` |
| Assignment 2 | `c6-workshop/assignment-02-led-components` |
| Assignment 3 | `c6-workshop/assignment-03-wifi-station` |
| Assignment 4 | `c6-workshop/assignment-04-nvs` |
| Assignment 5 | `c6-workshop/assignment-05-wifi-provisioning` |
| Assignment 6 (protocols + TLS) | `c6-workshop/assignment-06-protocols-tls` |
| Assignment 7 (LP core) | `c6-workshop/assignment-07-low-power` |

For **Assignment 6**, set Wi-Fi credentials in `idf.py menuconfig` → **Workshop example: Wi-Fi** before flashing.

---

## Příklady k workshopu ESP32-C6 (CZ)

Struktura `c6-workshop/` odpovídá kapitolám workshopu **1–7** na webu. U **úkolu 6 (Protokoly)** nastavte Wi-Fi v menuconfig (**Workshop example: Wi-Fi**) před flashnutím.
