# Physik LK für den Casio ClassPad II mit Hollyhock 3

Die eigenständige HH3-Projektlinie der Physik-Lernapp für den ClassPad II.
Der Fokus liegt auf **Hollyhock 3**, **PythonExtra.hh3** und einem daran
angepassten Ladeweg für Wissen und Abbildungen.

> [!NOTE]
> Die App ist für **Hollyhock 3 mit PythonExtra.hh3** vorgesehen.
> Für Hollyhock 2 steht die separate
> [HH2-Version](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App)
> zur Verfügung.

## Downloads

| Paket | Verwendung |
| --- | --- |
| [ClassPad v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_classpad_v1_0_0.zip) | Genau drei App-Dateien für Hollyhock 3 / PythonExtra.hh3 |
| [PC-Vorschau v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_pc_v1_0_0.zip) | Gleiche App mit Grafiksimulation und Startdatei für Windows |
| [Prüfsummen](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/SHA256SUMS.txt) | SHA256 der ZIPs und des Paketmanifests |

Alle Downloads stehen im [HH3-Release v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/tag/v1.0.0).

## Funktionen

- **146 Theorieartikel**, **368 eindeutige Formeln** und **514 Wissenskapitel**.
- **167 Rechner- und Messreihenfunktionen** mit passenden Einheiten.
- Mechanik, Schwingungen, Elektrizität und Wechselstrom sowie Quanten- und Atomphysik.
- **39 fachliche Abbildungen** als Skizzen und Rasterbilder.
- Wissen und Bilder werden nach Bedarf geladen.

Die HH3-Version bietet optimierte Dateizugriffe und Speicherverwaltung.
Weitere technische Informationen stehen in [ROADMAP.md](ROADMAP.md).

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
liegen. **Eine zusätzliche `cinput.py`, `gint.py` oder separate Indexdatei
wird nicht benötigt.** Die native Grafikfunktionalität kommt aus PythonExtra.

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
erläutert die Zuordnung der Laufzeiten. Für dieses Projekt ist `.hh3`
vorgesehen, nicht die HH2-Datei `PythonExtra-hh2.bin`.

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

Verwende für die Installation das ClassPad-ZIP, nicht das komplette
Quellcode-Archiv von GitHub. README, Roadmap, Tests und Build-Werkzeuge
gehören nicht auf den Taschenrechner.

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

Das Hauptmenü enthält ausschließlich:

- Theorie
- Formeln nach Groessen
- Formeln nach Themen
- Physik-Rechner
- Beenden

Touch und Pfeiltasten dienen der Auswahl und Navigation. DEL beziehungsweise
Zurück führt zur vorherigen Ansicht. In Artikeln mit Bildern öffnet
`Abbildungen (EXE)` oder die EXE-Taste den Betrachter. Links/rechts wechselt
die Darstellung; Zurück stellt die bisherige Textposition wieder her.

## HH3-Ladeoptimierung

Die HH3-Version springt nach bestätigtem
`seek()`/`tell()` direkt zur Position und liest mit `readinto()` bis zu 4 KiB
pro Aufruf. Bei unbrauchbarer Positionierung wird die Datei neu geöffnet
und kontrolliert sequenziell gelesen.

Bei erfolgreichen Bildaufrufen entfallen laufende RAM-Abfragen; erzwungene
Speicherbereinigungen erfolgen gezielt vor und nach dem Bildpuffer.
Paketkennungen und Prüfsummen erkennen unpassende oder beschädigte Daten.
Es wird jeweils nur eine Abbildung geladen.

## Kompatibilität und Prüfstatus

Am ClassPad erprobte PythonExtra-Laufzeit:

```text
MicroPython v1.25.0-preview.602.gfca9b039f on 2026-02-17
sh7305 with sh-4a
```

Ein vollständiger Hardwaretest aller Funktionen und aller HH3-/PythonExtra-Versionen
liegt nicht vor. Geräte-Ladezeiten und Speicherverbrauch wurden nicht
standardisiert vermessen. PC-Tests ersetzen keine Prüfung am Taschenrechner.

Die Softwareprüfung umfasst **35 Testgruppen** für Wissenskapitel,
Rechnerfunktionen, Bilddarstellung, Navigation, fehlerhafte Dateien und
Release-Pakete. Alle 39 Abbildungen werden pixelgenau mit der Referenz
verglichen; die PC-Vorschau wird zusätzlich mit wiederholten Bildwechseln
und dem Beenden-Menü geprüft.

## Entwicklung und Tests

Die Entwicklungswerkzeuge bleiben im Quellcode-Repository, werden aber
**nicht in die Release-ZIPs oder die App eingebaut**. Der Produktionsbuild:

```text
py -3 tools/build_hh3.py --release 1.0.0
```

Die Ausgabe liegt getrennt unter `release-ready/v1_0_0`. Der Build lehnt
Diagnosecode und zusätzliche Paketdateien ab. Wissen und Bilder werden
bytegenau gegen die eingefrorene öffentliche Ausgangsbasis geprüft.

Tests ausführen (für Bildvergleiche wird Pillow benötigt):

```text
py -3 tools/run_tests.py
py -3 tools/check_preview.py
py -3 tools/check_preview.py --app release-ready/v1_0_0/pc-preview/physchem_final.py
```

Testberichte und Bildvergleiche bleiben unter `testbuild/hh3_preview`.
Optional kann für die Entwicklung weiterhin ein getrenntes Diagnosepaket
mit `py -3 tools/build_hh3.py --diagnostics` erzeugt werden. Nur dieses erhält
`HH3-Bildtest`, Vergleichsladewege und Messanzeigen; der Release enthält sie nicht.

## Hinweise

- Die HH3-Version ist eine Weiterentwicklung der
  [HH2-App](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App).
- Dieses Community-Projekt ist nicht mit Casio verbunden. Schul- und
  Prüfungsregeln beachten; keine Zusicherung für den Prüfungsmodus.
