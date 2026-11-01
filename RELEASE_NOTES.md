# Physik LK HH3 v1.0.0

Erster Release der eigenstaendigen Hollyhock-3-Projektlinie, 1. November 2026.
Die App benoetigt weiterhin **PythonExtra.hh3**; dies ist keine native C-/C++-Portierung.
Das Repository und seine Release-Downloads bleiben privat.

## Inhalt

- Optimierter HH3-Dateizugriff mit gepruefter Direktpositionierung, Lesebloecken
  bis 4 KiB und kontrolliertem sequenziellem Ersatzweg.
- Keine laufenden RAM-Abfragen bei erfolgreichen Bildaufrufen und weniger
  erzwungene Speicherbereinigungen.
- Keine Grafiktests, Diagnosemenues, versteckten Vergleichsladewege oder
  Testbilder in der App. Fachliche Bilder und verstaendliche Fehleranzeigen bleiben erhalten.
- Hauptmenue: Theorie, Formeln nach Groessen, Formeln nach Themen,
  Physik-Rechner, Beenden.
- Unveraenderter fachlicher Umfang: 146 Theorieartikel, 368 eindeutige Formeln,
  514 Wissenskapitel, 167 Rechner-/Messreihenfunktionen und 39 eigene Abbildungen.
- Keine privaten PDF-Ausschnitte oder Unterrichtsmaterialien.

## Downloads und Installation

**ClassPad:** `physik_hh3_classpad_v1_0_0.zip` entpacken und genau diese drei
Dateien gemeinsam in einen Ordner wie `physik_hh3` auf dem ClassPad kopieren:

```text
physchem_final.py
wissen.txt
bilder.dat
```

Alle drei Dateien zusammen ersetzen. Im gesamten Startpfad keine Leerzeichen,
Bindestriche oder Klammern verwenden. Geraet sicher auswerfen, PythonExtra
neu starten und `physchem_final.py` in der Dateiliste auswaehlen.
Auf dem ClassPad werden weder `cinput.py` noch `gint.py` benoetigt.

**PC:** `physik_hh3_pc_v1_0_0.zip` entpacken und unter Windows
`start_pc_preview.bat` starten. Python 3 mit Tkinter wird vorausgesetzt.
Die enthaltene `gint.py` ist ausschliesslich die PC-Grafiksimulation.

`SHA256SUMS.txt` enthaelt die ZIP- und Manifest-Pruefsummen;
`manifest.json` dokumentiert zusaetzlich die Pruefsummen der drei App-Dateien.
Die automatisch von GitHub angebotenen Quellcode-Archive sind nicht das ClassPad-Installationspaket.

## Pruefstatus

Der Nutzer bestaetigt die Funktion des optimierten Builds auf seinem ClassPad
und subjektiv kuerzere Bildladezeiten als beim bisherigen HH2-Build.
Gemeldete Laufzeit: MicroPython v1.25.0-preview.602.gfca9b039f vom 17.02.2026,
sh7305 mit sh-4a. Genaue Ladezeiten oder ein gemessener Beschleunigungsfaktor
liegen nicht vor; ein vollstaendiger Hardwaretest aller Funktionen wird nicht behauptet.

Die Softwarepruefung umfasst 35 Testgruppen, alle Wissenskapitel,
Rechnerregressionen, 39 pixelgleiche Bildansichten und gezielte Release-Pruefungen.
Die PC-Vorschau prueft vollstaendige Bildwechsel, begrenzte Bildreferenzen und Beenden.
Die drei App-Dateien im Release stimmen bytegenau mit dem geprueften Hauptstand ueberein.

Die HH2-Projektlinie, ihre Tags und Download-Pakete bleiben unveraendert.
