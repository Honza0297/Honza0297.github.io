# ESP-IDF workshop examples — ESP32-C5

Reference ESP-IDF projects aligned with the **published** [ESP32-C5 workshop](../../content/workshops/esp-idf-with-esp32-c5/_index.md) chapters (introduction + assignments **1–6**). Assignment **6** in that workshop is the **low-power / LP core** chapter.

## Environment

From a shell where ESP-IDF is installed, you should have `idf.py` on your `PATH`. If you rely on the Espressif installer helpers, a typical sequence is:

```bash
idfinstall && idfexport
```

Then:

```bash
./build-all.sh
```

This sets the target to **esp32c5** and builds each project under `c5-workshop/` and the optional parity project under `c5-extra/`.

## Layout (no missing numbers for the workshop track)

| Workshop chapter | Directory |
| --- | --- |
| (after intro / env ready) | `c5-workshop/assignment-01-install-verify` |
| Assignment 2 | `c5-workshop/assignment-02-led-components` |
| Assignment 3 | `c5-workshop/assignment-03-wifi-station` |
| Assignment 4 | `c5-workshop/assignment-04-nvs` |
| Assignment 5 | `c5-workshop/assignment-05-wifi-provisioning` |
| Assignment 6 (LP core) | `c5-workshop/assignment-06-low-power` |

### Optional: protocols + TLS (same idea as ESP32-C6 workshop assignment 6)

The C5 workshop text skips the dedicated protocols chapter, but you can still build the **HTTPS + ESP x509 certificate bundle** flow for **ESP32-C5** here:

| Role | Directory |
| --- | --- |
| Parity with **ESP32-C6** workshop *Assignment 6: Protocols* | `c5-extra/c6-ch06-protocols-tls-parity` |

Set Wi-Fi SSID and password via `idf.py menuconfig` → **Workshop example: Wi-Fi** before flashing.

---

## Příklady k workshopu ESP32-C5 (CZ)

Adresáře pod `c5-workshop/` odpovídají **oficiálnímu** workshopu (úvod + úkoly **1–6** na webu). Úkol **6** na webu je kapitola o **low-power / LP jádru**.

### Volitelně: protokoly a TLS

Text workshopu pro C5 záměrně vynechává samostatný úkol o protokolech, ale kompletní příklad **HTTPS + ESP x509 certificate bundle** pro čip **ESP32-C5** je v:

`c5-extra/c6-ch06-protocols-tls-parity`

…tj. stejný typ úlohy jako **úkol 6 (Protokoly)** u verze workshopu pro **ESP32-C6**. Před flashnutím nastavte SSID a heslo v menuconfig (**Workshop example: Wi-Fi**).
