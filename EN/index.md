---
title: "[EN] Workshop: ESP-IDF and ESP32-C6"
date: 2024-09-30T00:00:00+01:00
tags: ["Workshop", "ESP-IDF", "ESP32-C6", "Espressif IDE"]

---

Welcome to the Espressif IoT Development Framework (ESP-IDF) workshop!

## About this workshop

By participating in this workshop, you will gain a deeper understanding of how the Espressif IoT Development Framework (ESP-IDF) and ESP32-C6 *System on a Chip* (SoC) work. Through many practical tasks that will help you with various basic skills needed for programming (not only) ESP32-C6, you will learn to create even better and more efficient IoT applications for your projects!

Specifically, in this workshop we will go through the installation of the ESP-IDF framework and ESP-IDF plugin for VSCode. Then we will move on to creating our own project, which we will try to build, upload and debug. In the following chapters, we will learn how to use components in your projects, blink with addressable LEDs, configure ESP-IDF, connect to Wi-Fi and finally some tricks for working with the low-power core that is part of the ESP32-C6 SoC.

> This workshop will certainly bring a lot of interesting information even to those who use other chips from the ESP32 family. However, keep in mind that not all chips support everything that ESP32-C6 supports!

## Companion reference projects

Complete ESP-IDF projects for each assignment are included in this repository under
[examples/C6/](../examples/C6/). See [examples/C6/README.md](../examples/C6/README.md) for
usage instructions and `build-all.sh` to build everything in one go.

## Agenda

If you have met the [prerequisites](#prerequisites), we can start with the individual chapters:

- [Introduction: ESP-IDF Presentation](introduction/): Initial introduction to our ESP-IDF framework, followed by basic information about the ESP32-C6 chip and the devkit on which this tutorial will be demonstrated.
- [Assignment 1: Installing ESP-IDF in VSCode](assignment-1): Installing the necessary tools to start development.
- [Assignment 2: Creating a new project with Components](assignment-2): How to efficiently create, structure and organize a project.
- [Assignment 3: Connecting to Wi-Fi](assignment-3): The absolute basics for most IoT projects.
- [Assignment 4: Testing NVS (Non-Volatile Storage)](assignment-4): Storing persistent data.
- [Assignment 5: Wi-Fi provisioning](assignment-5): More detailed work with Wi-Fi: setting up and configuring Wi-Fi networks.
- [Assignment 6: Protocols](assignment-6): Demonstration of several communication protocols that ESP32-C6 supports, including TLS implementation for secure communication.
- [Assignment 7: Experiments with low-power core](assignment-7): Demonstration of working with low-power core for applications where energy consumption is an important factor.


## Prerequisites

To handle this workshop, you will need both hardware and software equipment.

Required hardware:

- Computer with Linux, Windows or macOS operating system
- ESP32-C6-DevKitC or ESP32-C6-DevKitM
- USB cable (supporting power + data) compatible with the devkit above
- Visual Studio Code with ESP-IDF plugin (installation is described in the next section)

Required software:

- [Visual Studio Code](https://code.visualstudio.com/download)
- [ESP-IDF plugin for VS Code](https://github.com/espressif/vscode-esp-idf-extension?tab=readme-ov-file#how-to-use)
- **ESP BLE Prov** app on your mobile phone:
    - Android: [ESP BLE Prov](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
    - iOS: [ESP BLE Prov](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

## Time Requirements

{{< alert icon="mug-hot" >}}
**Estimated time: 180 min**
{{< /alert >}}


## Feedback

If you have any feedback about the workshop, feel free to start a new [discussion on GitHub](https://github.com/espressif/developer-portal/discussions).


## Conclusion

Everyone at Espressif hopes that the workshop will bring you knowledge that will serve as a solid foundation for your future projects. Thank you for the time and effort you devoted to the workshop, and we look forward to the projects you will create with our chips in the future!

---