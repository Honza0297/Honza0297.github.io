---
title: "Workshop: ESP-IDF a ESP32-C5 - Úkol 1: Instalace ESP-IDF"
date: 2024-09-30T00:00:00+01:00
layout: default
---

## Úkol 1: Instalace ESP-IDF a VSCode

---

Pro tento workshop budete potřebovat [Visual Studio Code](https://code.visualstudio.com/) a nainstalovaný framework ESP-IDF. Prvním úkolem tedy bude nainstalovat si všechny potřebné nástroje.

K instalaci ESP-IDF použijeme [EIM](https://docs.espressif.com/projects/idf-im-ui/en/latest/) (Espressif Installation Manager).

### Instalace pro Windows

1. Nainstalujte [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Stiskněte klávesu Windows + R a do objevivšího se okénka napište powershell
    * Po zobrazení terminálu zadejte: `winget install Espressif.eim`
2. Nainstalujte ESP-IDF pomocí EIM
    * Spusťte EIM z nabídky start nebo pomocí ikony na ploše
    * Na uvítací obrazovce klikněte na tlačítko `Start Installation` a poté `Custom installation` a nechte se průvodcem vést. Na stránce s výběrem verze zvolte verzi IDF 5.5.x
    * Instalátor vám v průběhu nabídne i nainstalování gitu nebo pythonu, pokud vám v počítači zatím chybí.
    * Po dokončení instalace se vraťte na stránku instalátoru s přehledem verzí a použijte tlačítko `Install Drivers` ve spodní části obrazovky, které pro vás nainstaluje potřebné ovladače.
3. Nainstalujte [Visual Studio Code](https://code.visualstudio.com/download)
4. Doporučujeme nainstalovat do VS Code rozšíření ESP-IDF kvůli našeptávání kódu, ale pro build a flashování budeme v tomto workshopu výhradně používat **ESP-IDF Terminal**.

### Instalace pro Linux a Mac

1. Nainstalujte [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Pokud zvolíte instalaci pomocí balíčkovacího systému (brew, apt, rpm), vyřeší za vás i všechny [prerekvizity](https://docs.espressif.com/projects/idf-im-ui/en/latest/prerequisites.html).
    * Pokud zvolíte CLI verzi, zavolejte `eim install -i v5.5.4`
    * Pokud zvolíte GUI verzi, klikněte na uvítací obrazovce na tlačítko `Start Installation` a poté `Custom installation` a nechte se průvodcem vést. Na stránce s výběrem verze zvolte verzi IDF 5.5.x
2. Nainstalujte [Visual Studio Code](https://code.visualstudio.com/download)

### Instalace ESP BLE Prov

Během jednoho z úkolů vyzkoušíme také Wi-Fi provisioning. K tomu budeme potřebovat aplikaci, přes kterou našemu čipu ESP přes Bluetooth sdělíme SSID a heslo k síti.

Aplikaci buď vyhledáte v obchodě s aplikacemi příslušného systému, nebo přes odkazy níže:

- Android: [ESP BLE Provisioning](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
- iOS: [ESP BLE Provisioning](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

### ESP-IDF Terminal

V tomto workshopu budeme s ESP-IDF pracovat přímo z příkazové řádky (CLI), a ne pomocí tlačítek v GUI. Díky tomu lépe pochopíte, co se děje „pod kapotou“, a v budoucnu si budete moci úlohy zautomatizovat.

Než budete moci používat nástroje ESP-IDF, jako je `idf.py`, musí být prostředí ESP-IDF v terminálu *aktivováno*. Vyberte si kteroukoli z následujících metod.

#### Možnost 1: Dashboard v EIM (všechny systémy)

1. Otevřete aplikaci EIM (`eim`) z nabídky start / spouštěče aplikací, podle vašeho systému.
2. Klikněte na `Manage Installations` a poté na `Open Dashboard`.
3. Vyberte verzi ESP-IDF, kterou chcete používat (v5.5.x), a klikněte na `Open IDF Terminal`.
4. Otevře se terminál s aktivovaným prostředím ESP-IDF. Ověřte to příkazem `idf.py --version`.

#### Možnost 2: Zástupce na ploše (Windows)

EIM vytvoří na ploše zástupce pro každou nainstalovanou verzi, např. `IDF_v5.5.4_Powershell`. Dvojklikem na něj otevřete PowerShell s již aktivovaným ESP-IDF.

#### Možnost 3: Aktivační skript (Linux a macOS)

Na konci instalace vypíše EIM CLI příkaz podobný tomu níže. Spusťte ho v libovolném terminálu a ESP-IDF se v dané relaci terminálu aktivuje (**místo `<version>` doplňte svou konkrétní verzi**):

```
source "$HOME/.espressif/tools/activate_idf_v<version>.sh"
```

Přesnou cestu a verzi vypíše EIM na konci instalace. Tento příkaz je nutné spustit v každém novém terminálu.

#### Možnost 4: Rozšíření ESP-IDF ve VS Code

Pokud jste si nainstalovali rozšíření ESP-IDF, stiskněte `Ctrl + Shift + P` (nebo `Cmd + Shift + P`), napište **ESP-IDF: Open ESP-IDF Terminal** a stiskněte Enter.

V každém případě můžete ověřit, že prostředí funguje, spuštěním `idf.py --version` v terminálu. Pokud se příkaz nenajde, prostředí v daném terminálu aktivované není.

Nyní byste měli mít funkční prostředí ESP-IDF. Pojďme na náš první projekt!

[Úkol 2: Základy: Hello World & LED](../assignment-2/)
