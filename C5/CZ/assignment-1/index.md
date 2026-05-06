---
title: "Workshop: ESP-IDF a ESP32-C5 - Úkol 1"
date: 2024-09-30T00:00:00+01:00
layout: default
---

## Úkol 1: Instalace ESP-IDF a VSCode s ESP-IDF pluginem

---

Abyste byli schopní zvládnout všechny úkoly v tomto workshopu, budete potřebovat [Visual Studio Code](https://code.visualstudio.com/) a nainstalovaný framework ESP-IDF. Prvním úkolem tedy bude nainstalovat si všechny potřebné nástroje.


K instalaci ESP-IDF použijte náš nový instalátor [EIM](https://docs.espressif.com/projects/idf-im-ui/en/latest/) a rozšíření do VSCode nainstalujte podle návodu na [githubových stránkách ESP-IDF pluginu pro VS Code](https://github.com/espressif/vscode-esp-idf-extension?tab=readme-ov-file#how-to-use).

**Windows:**

### Instalace pro Windows

1. Nainstalujte [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Stiskněte klávesu Windows + R a do objevivšího se okénka napište powershell
    * Po zobrazení terminálu zadejte `winget install Espressif.eim`
2. Nainstalujte ESP-IDF pomocí EIM
    * Spusťte EIM z nabídky start nebo pomocí ikony na ploše
    * Na uvítací obrazovce klikněte na tlačítko `Start Installation` a poté `Custom installation` a nechte se průvodcem vést. Na stránce s výběrem verze zvolte verzi IDF 5.5.4
    * Instalátor vám v průběhu nabídne i nainstalování gitu nebo pythonu, pokud vám v počítači zatím chybí.
    * Po dokončení instalace se vraťte na stránku instalátoru s přehledem verzí a použijte tlačítko `Install Drivers`, které pro vás nainstaluje potřebné ovladače.

3. Nainstalujte [Visual Studio Code](https://code.visualstudio.com/download)
4. Ve VS Code otevřete **Extensions** (Ctrl + Shift + X nebo ⇧ + ⌘ + X)
5. Najděte [ESP-IDF plugin](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) a nainstalujte ho
6. Počkejte, až se vám v levém panelu objeví ikona Espressif, a klikněte na ni.
7. Rozšíření si samo najde nainstalované ESP-IDF

**Linux a Mac:**

### Instalace pro Linux a Mac

1. Nainstalujte [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Pokud zvolíte instalaci pomocí balíčkovacího systému (brew, apt, rpm), vyřeší za vás i všechny [prerekvizity](https://docs.espressif.com/projects/idf-im-ui/en/latest/prerequisites.html).
    * Pokud zvolíte CLI verzi, zavolejte `eim install -i v5.5.4`
    * Pokud zvolíte GUI verzi, klikněte na uvítací obrazovce na tlačítko `Start Installation` a poté `Custom installation` a nechte se průvodcem vést. Na stránce s výběrem verze zvolte verzi IDF 5.5.4


2. Nainstalujte [Visual Studio Code](https://code.visualstudio.com/download)
3. Ve VS Code otevřete **Extensions** (Ctrl + Shift + X nebo ⇧ + ⌘ + X)
4. Najděte [ESP-IDF plugin](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) a nainstalujte ho
5. Počkejte, až se vám v levém panelu objeví ikona Espressif, a klikněte na ni.
6. Rozšíření si samo najde nainstalované ESP-IDF

### Instalace ESP BLE Prov

Během jednoho z úkolů vyzkoušíme také Wi-Fi provisioning. K tomu budeme potřebovat aplikaci, přes kterou našemu ESPčku sdělíme SSID a heslo k síti.

Aplikaci buď vyhledáte v appstoru příslušného systému, nebo přes odkazy níže:

- Android: [ESP BLE Provisioning](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
- iOS: [ESP BLE Provisioning](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

### VSCode, ESP-IDF plugin a samotné ESP-IDF

Možná jste si všimli, že během instalace ESP-IDF pluginu jste vybírali i verzi samotného ESP-IDF. Jak spolu tedy souvisí ESP-IDF plugin a samotné ESP-IDF?

ESP-IDF je **samostatný framework**, který lze používat i bez VSCode a pluginu. Ovládá se z příkazové řádky (nemá tedy žádné GUI) a obsahuje všechnu logiku pro build, flashování i monitorování aplikace. Když tedy zmáčknete např. tlačítko *Build* v *ESP-IDF Exploreru* (nebojte, v dalších lekcích si vysvětlíme, co to znamená), nakonec se stejně zavolá příkaz `idf.py build`.

ESP-IDF VSCode plugin je **wrapper/adaptér**, který zpřístupňuje funkcionalitu ESP-IDF ve VSCode. Jinak řečeno, propojuje VSCode a framework ESP-IDF: vytváří GUI v podobě *ESP-IDF Exploreru* a zároveň se stará o to, aby tlačítka plnila svou funkci, výstup příkazů se korektně zobrazoval ve VSCode, aby se konfigurace frameworku správně projevila a podobně. Ke svému fungování ale potřebuje právě i framework, který běží v pozadí.

> **Poznámka:** Samotný ESP-IDF framework se dá používat i jinak, než jen s pluginem do VSCode. Podporu pro něj má [Clion](https://developer.espressif.com/blog/clion/), [Eclipse](https://developer.espressif.com/blog/eclipse-plugin-for-esp-idf/) a v neposlední řadě ho můžete používat samostatně, [přímo z příkazové řádky](https://developer.espressif.com/blog/getting-started-with-esp-idf/).
>
> Nicméně, v tomto workshopu budeme používat výhradně **plugin do VSCode**.

Nyní byste měli mít funkční ESP-IDF plugin pro VSCode a nastavené ESP-IDF. Můžeme se tedy pustit do druhé části tutoriálu:

[Úkol 2: Vytváření projektu a Komponenty](../assignment-2)
