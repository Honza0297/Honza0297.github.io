# ESP-IDF workshop examples — ESP32-C6

Reference ESP-IDF projects for the ESP32-C6 workshop.

From this directory, `./build-all.sh` sets the target to **esp32c6** and builds each project.

## Layout

| Workshop chapter | Directory |
| --- | --- |
| Assignment 1: Install ESP-IDF | [`assignment-01-install-verify`](assignment-01-install-verify/) |
| Assignment 2: LED | [`assignment-02-led-components`](assignment-02-led-components/) |
| Assignment 3: Wi-Fi & provisioning | [`assignment-03-wifi-provisioning`](assignment-03-wifi-provisioning/) |
| Assignment 4: OTA updates | [`assignment-04-ota`](assignment-04-ota/) |
| Assignment 5: ESP-NOW | [`assignment-05-esp-now`](assignment-05-esp-now/) |
| Assignment 6: LP-core | [`assignment-06-low-power`](assignment-06-low-power/) |

On the website, open a project folder to read the files, or download the `.zip` linked beside it.

```bash
idfinstall && idfexport
./build-all.sh
```
