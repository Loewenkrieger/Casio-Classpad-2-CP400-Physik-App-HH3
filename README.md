# Physik LK für den Casio ClassPad II mit Hollyhock 3

**Physik LK** ist eine Lern- und Formel-App für den **Casio ClassPad II
(fx-CP400)** mit Hollyhock 3. Sie verbindet Theorieartikel, eine Formelsuche,
einen Physik-Rechner und passende Abbildungen in einer Oberfläche für
Touchscreen und Tasten.

> [!NOTE]
> Die App ist für **Hollyhock 3 mit PythonExtra.hh3** vorgesehen.
> Für Hollyhock 2 steht die separate
> [HH2-Version](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App)
> zur Verfügung.

## Downloads

| Paket | Verwendung |
| --- | --- |
| [ClassPad v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_classpad_v1_0_0.zip) | Installation auf dem Taschenrechner |
| [PC-Vorschau v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_pc_v1_0_0.zip) | App am PC ausprobieren, ohne ClassPad |
| [Prüfsummen](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/SHA256SUMS.txt) | SHA256 der ZIPs und des Paketmanifests |

Alle Downloads stehen im [HH3-Release v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/tag/v1.0.0).

## Funktionen

- **Theorie nachschlagen:** 146 Artikel zu Mechanik, Schwingungen,
  Elektrizität, Wechselstrom sowie Quanten- und Atomphysik.
- **Formeln finden:** 368 eindeutige Formeln, sortiert nach Größen und Themen.
- **Werte berechnen:** 167 Rechner- und Messreihenfunktionen mit passenden Einheiten.
- **Zusammenhänge veranschaulichen:** 39 fachliche Skizzen und Rasterbilder,
  direkt aus den zugehörigen Artikeln erreichbar.
- **Speicherschonend lesen:** Die 514 Wissenskapitel und die Abbildungen
  werden bei Bedarf aus den Datendateien geladen.

## Voraussetzungen

- Ein ClassPad II mit funktionierendem Hollyhock 3.
- Die dazu passende PythonExtra-Datei **`PythonExtra.hh3`**.

Hollyhock und PythonExtra werden nicht mit dieser App mitgeliefert.
Die HH2- und HH3-Pakete sind für ihre jeweilige Laufzeit vorgesehen und
sollten nicht untereinander ausgetauscht werden.

## Benötigte App-Dateien

```text
physchem_final.py
wissen.txt
bilder.dat
```

| Datei | Inhalt |
| --- | --- |
| `physchem_final.py` | Oberfläche, Navigation und Physik-Rechner |
| `wissen.txt` | Theorie, Formeln und Verzeichnisse |
| `bilder.dat` | Fachliche Skizzen und Rasterbilder |

Die drei Dateien gehören zusammen und müssen auf dem Gerät im selben Ordner
liegen. PythonExtra stellt Grafik und Eingabe bereit; zusätzliche
Python-Bibliotheken sind auf dem Taschenrechner nicht erforderlich.

## 1. Hollyhock 3 und PythonExtra vorbereiten

> [!WARNING]
> Eine Hollyhock-Installation verändert das Gerät und erfolgt auf eigene
> Gefahr. Kompatibilität, Warnungen und Installationsschritte der
> [offiziellen HH3-Anleitung](https://classpaddev.github.io/wiki/hh3/getting-started/install/)
> sind maßgeblich. Wenn HH3 bereits funktioniert, ist keine Neuinstallation nötig.

1. Stelle sicher, dass Hollyhock 3 und sein Launcher funktionieren.
2. Lade die zu HH3 passende Datei `PythonExtra.hh3` von der
   [offiziellen PythonExtra-Downloadseite](https://classpaddev.github.io/python/#download).
3. Kopiere sie per USB in das Hauptverzeichnis des ClassPad-Speichers und
   wirf das Gerät sicher aus.
4. Öffne über das Systemmenü den Hollyhock-Launcher und starte PythonExtra.

Die [PythonExtra-Installationsanleitung](https://classpaddev.github.io/wiki/python/installation-guide/)
erläutert die Zuordnung der Laufzeiten. Auf Hollyhock 3 wird `PythonExtra.hh3`
benötigt, nicht die HH2-Datei `PythonExtra-hh2.bin`.

## 2. App auf dem ClassPad installieren

1. Lade das **ClassPad-ZIP** aus dem HH3-Release herunter und entpacke es.
2. Kopiere `physchem_final.py`, `wissen.txt` und `bilder.dat` gemeinsam in
   einen Ordner wie `physik_hh3` auf dem ClassPad. Vorhandene drei Dateien zusammen ersetzen.
3. Wirf das Gerät sicher aus und starte PythonExtra neu.
4. Wähle `physchem_final.py` in der Dateiliste aus.

> [!IMPORTANT]
> Verwende im gesamten Startpfad keine Leerzeichen, Bindestriche oder Klammern.
> Der automatisch entpackte GitHub-Repository-Ordner enthält Bindestriche:
> Starte die App deshalb nicht direkt daraus. Unterstriche wie in
> `physik_hh3` sind geeignet. Alle drei Dateien immer gemeinsam austauschen.

Das **ClassPad-ZIP** enthält alle benötigten App-Dateien.
Die von GitHub zusätzlich angebotenen Quellcode-Archive sind für die
Entwicklung bestimmt, nicht für die Installation auf dem Taschenrechner.

## 3. PC-Vorschau

1. Installiere Python 3 mit Tkinter auf deinem PC.
2. Lade das **PC-ZIP** aus demselben HH3-Release herunter und entpacke es.
3. Starte unter Windows `start_pc_preview.bat`.

Alternativ im entpackten PC-Ordner:

```text
py -3 physchem_final.py
```

Die PC-Vorschau enthält dieselbe App. `gint.py` simuliert
die Grafikfunktionen ausschließlich am PC und gehört nicht auf den ClassPad.

## Bedienung

| Menüpunkt | Funktion |
| --- | --- |
| Theorie | Themen auswählen und Erklärungen lesen |
| Formeln nach Groessen | Formeln zu einer gesuchten physikalischen Größe finden |
| Formeln nach Themen | Formeln eines Themenbereichs durchsuchen |
| Physik-Rechner | Formel auswählen, Werte eingeben und Ergebnis mit Einheit erhalten |
| Beenden | Die App schließen |

Touch und Pfeiltasten dienen der Auswahl und Navigation. DEL beziehungsweise
Zurück führt zur vorherigen Ansicht. In Artikeln mit Bildern öffnet
`Abbildungen (EXE)` oder die EXE-Taste den Betrachter. Links/rechts wechselt
die Darstellung; Zurück stellt die bisherige Textposition wieder her.

## Kompatibilität und Prüfstatus

Am ClassPad erprobte PythonExtra-Laufzeit:

```text
MicroPython v1.25.0-preview.602.gfca9b039f on 2026-02-17
sh7305 with sh-4a
```

Ein vollständiger Hardwaretest aller Funktionen und aller HH3-/PythonExtra-Versionen
liegt nicht vor. Geräte-Ladezeiten und Speicherverbrauch wurden nicht
standardisiert vermessen. PC-Tests ersetzen keine Prüfung am Taschenrechner.

## Fehler melden und mitwirken

Fehlerberichte und Verbesserungsvorschläge sind über die
[GitHub-Issues](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/issues)
möglich. Hilfreich sind die App- und PythonExtra-Version, die betroffene
Funktion und eine kurze Beschreibung der Schritte bis zum Fehler.

Build-Anleitung, Testbefehle und Projektstruktur stehen in
[DEVELOPMENT.md](DEVELOPMENT.md). Technische Hintergründe und offene
Entwicklungsschwerpunkte beschreibt die [Roadmap](ROADMAP.md).

## Hinweise

- Die HH3-Version ist eine Weiterentwicklung der
  [HH2-App](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App).
- Dieses Community-Projekt ist nicht mit Casio verbunden. Schul- und
  Prüfungsregeln beachten; keine Zusicherung für den Prüfungsmodus.
