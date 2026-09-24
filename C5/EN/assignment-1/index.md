---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 1: Install ESP-IDF"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 1: Install ESP-IDF and VSCode

---

You will need [Visual Studio Code](https://code.visualstudio.com/) and the installed ESP-IDF
framework for this workshop. The first task will therefore be to install all the necessary tools.

To install ESP-IDF, we will use the [EIM](https://docs.espressif.com/projects/idf-im-ui/en/latest/) (Espressif Installation Manager).

### Windows Installation

1. Install [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Press Windows key + R and type powershell in the dialog box that appears
    * After the terminal appears, enter: `winget install Espressif.eim`
2. Install ESP-IDF using EIM
    * Run EIM from the start menu or desktop icon
    * On the welcome screen, click `Start Installation` and then `Custom installation` and follow the wizard. On the version selection page, choose IDF version 5.5.x
    * During installation, the installer will also offer to install git or python if they are not already on your computer.
    * After installation is complete, return to the installer's version manager page and use the `Install Drivers` button in the bottom of the screen to install the necessary drivers.
3. Install [Visual Studio Code](https://code.visualstudio.com/download)
4. We recommend installing the ESP-IDF extension in VS Code for code completion, but we will strictly use the **ESP-IDF Terminal** for building and flashing in this workshop.

### Linux and Mac Installation

1. Install [EIM](https://dl.espressif.com/dl/eim/index.html)
    * If you choose installation via a package manager (brew, apt, rpm), it will handle all [prerequisites](https://docs.espressif.com/projects/idf-im-ui/en/latest/prerequisites.html) for you.
    * If you choose the CLI version, call `eim install -i v5.5.4`
    * If you choose the GUI version, click the `Start Installation` button on the welcome screen and then `Custom installation` and follow the wizard. On the version selection page, choose IDF version 5.5.x
2. Install [Visual Studio Code](https://code.visualstudio.com/download)

### Installing ESP BLE Prov

During one of the tasks, we will also try Wi-Fi provisioning. For this we will need an application
through which we will tell our ESP chip the SSID and password for the network via Bluetooth.

You can either search for the app in the app store of the respective system, or through the links
below:

- Android: [ESP BLE Provisioning](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
- iOS: [ESP BLE Provisioning](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

### The ESP-IDF Terminal

In this workshop, we will interact with ESP-IDF directly through the command line (CLI) rather than relying on GUI buttons. This gives you a better understanding of what happens under the hood and allows you to automate tasks in the future.

If you installed the ESP-IDF extension in VS Code, you can open an ESP-IDF Terminal by pressing `Ctrl + Shift + P` (or `Cmd + Shift + P`), typing **ESP-IDF: Open ESP-IDF Terminal**, and pressing Enter.

Alternatively, you can open a standard terminal and run the `export.sh` (or `export.bat`) script located in your ESP-IDF installation directory to set up the environment variables.

You should now have a working ESP-IDF environment. Let's start with our first project!

[Assignment 2: Basics: Hello World & LED](../assignment-2/)
