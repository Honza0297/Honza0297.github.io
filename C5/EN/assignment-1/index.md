---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 1: Install ESP-IDF"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 1: Install ESP-IDF and VSCode

---

You will need [Visual Studio Code](https://code.visualstudio.com/) and the installed ESP-IDF framework for this workshop. The first task will therefore be to install all the necessary tools.

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

Before you can use ESP-IDF tools such as `idf.py`, the ESP-IDF environment must be *activated* in your terminal. Pick whichever of the following methods suits you best.

#### Option 1: EIM dashboard (all systems)

1. Open the EIM application (`eim`) from the start menu / applications launcher, depending on your OS.
2. Click `Manage Installations` and then `Open Dashboard`.
3. Select the ESP-IDF version you want to use (v5.5.x) and click `Open IDF Terminal`.
4. A terminal with the activated ESP-IDF environment opens. Verify it by running `idf.py --version`.

#### Option 2: Desktop shortcut (Windows)

EIM creates a desktop shortcut for each installed version, e.g. `IDF_v5.5.4_Powershell`. Double-click it to open a PowerShell session with ESP-IDF already activated.

#### Option 3: Activation script (Linux and macOS)

At the end of the installation, EIM CLI prints a command like the one below. Run it in any terminal to activate ESP-IDF in that terminal session (**fill in your specific version instead of ``<version>``**):

```
source "$HOME/.espressif/tools/activate_idf_v<version>.sh"
```

The exact path and version are printed by EIM at the end of the installation. You need to run this command in every new terminal.

#### Option 4: ESP-IDF extension in VS Code

If you installed the ESP-IDF extension, press `Ctrl + Shift + P` (or `Cmd + Shift + P`), type **ESP-IDF: Open ESP-IDF Terminal**, and press Enter.

In every case, you can check that the environment works by running `idf.py --version` in the terminal. If the command is not found, the environment is not activated in that terminal.

You should now have a working ESP-IDF environment. Let's start with our first project!

[Assignment 2: Basics: Hello World & LED](../assignment-2/)
