# Physik LK HH3 v1.0.0

Lern- und Formel-App für den **Casio ClassPad II (fx-CP400)** mit
**Hollyhock 3 und PythonExtra.hh3**. Veröffentlicht am 1. November 2026.

## Funktionen

- 146 Theorieartikel und 368 eindeutige Formeln zu Mechanik, Schwingungen,
  Elektrizität, Wechselstrom sowie Quanten- und Atomphysik.
- 167 Rechner- und Messreihenfunktionen mit passenden Einheiten.
- 39 fachliche Skizzen und Rasterbilder, erreichbar aus den zugehörigen Artikeln.
- Formelsuche nach Größen und Themen, bedienbar über Touchscreen und Tasten.
- Bedarfsgesteuertes Laden von Wissen und Bildern mit optimierten HH3-Dateizugriffen.

## Installation auf dem ClassPad

`physik_hh3_classpad_v1_0_0.zip` entpacken und die drei Dateien gemeinsam
in einen Ordner wie `physik_hh3` auf dem ClassPad kopieren:

```text
physchem_final.py
wissen.txt
bilder.dat
```

Im gesamten Startpfad keine Leerzeichen, Bindestriche oder Klammern verwenden.
Gerät sicher auswerfen, PythonExtra neu starten und `physchem_final.py`
in der Dateiliste auswählen. Bei einem Update alle drei Dateien gemeinsam ersetzen.
Die App benötigt keine zusätzliche `cinput.py` oder `gint.py` auf dem Taschenrechner.

## PC-Vorschau

`physik_hh3_pc_v1_0_0.zip` entpacken und unter Windows
`start_pc_preview.bat` starten. Voraussetzung ist Python 3 mit Tkinter.
Die enthaltene `gint.py` simuliert die Grafikfunktionen am PC.

## Prüfsummen

`SHA256SUMS.txt` enthält die ZIP- und Manifest-Prüfsummen.
`manifest.json` enthält zusätzlich die Prüfsummen der drei App-Dateien.
Für die Installation das ClassPad-ZIP verwenden, nicht die automatisch
von GitHub angebotenen Quellcode-Archive.

## Kompatibilität

Am ClassPad erprobte Laufzeit: MicroPython `v1.25.0-preview.602.gfca9b039f`
vom 17.02.2026, sh7305 mit sh-4a. Ein vollständiger Hardwaretest aller
Funktionen und standardisierte Ladezeit- oder RAM-Messungen liegen nicht vor.

Die [README](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3#readme)
enthält die vollständige Installation und Bedienung. Für Hollyhock 2 steht die separate
[HH2-App](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App) zur Verfügung.
