# Entwicklung

Diese Anleitung richtet sich an Beiträge zum Quellcode, eigene Builds und
die Fehlersuche. Für die Installation der App sind die fertigen Pakete aus
den [Releases](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases)
und die [Installationsanleitung](README.md#2-app-auf-dem-classpad-installieren) vorgesehen.

## Voraussetzungen

- Python 3 für Build-Werkzeuge und Tests.
- Pillow für pixelgenaue Bildvergleiche in den automatisierten Tests.
- Tkinter für die interaktive PC-Vorschau und deren Oberflächenprüfung.
- ClassPad II mit Hollyhock 3 und PythonExtra.hh3 für Gerätetests.

Alle folgenden Befehle werden im Projektverzeichnis ausgeführt.
Unter Windows kann Python 3 mit `py -3` aufgerufen werden.

## Projektstruktur

| Pfad | Aufgabe |
| --- | --- |
| `physchem_final.py` | App-Oberfläche, Dateizugriffe und Rechnerfunktionen |
| `wissen.txt` | Indizierte Theorie- und Formeldatensätze |
| `bilder.dat` | Indizierte Vektor- und Rasterabbildungen |
| `tools/build_hh3.py` | ClassPad- und PC-Pakete erzeugen |
| `tools/gint.py` | PC-Grafiksimulation |
| `tools/check_preview.py` | Bildwechsel und Beenden in der PC-Vorschau prüfen |
| `tools/hh3_diagnostics.py` | Vergleichsmessungen für Diagnosebuilds |
| `tests/` | Automatisierte Tests und eingefrorene HH2-Referenz |

## Release-Pakete bauen

```text
py -3 tools/build_hh3.py --release 1.0.0
```

Ausgabe unter `release-ready/v1_0_0`:

- `classpad/`: die drei App-Dateien für den Taschenrechner.
- `pc-preview/`: App mit Grafiksimulation und Windows-Startdatei.
- `physik_hh3_classpad_v1_0_0.zip` und `physik_hh3_pc_v1_0_0.zip`.
- `manifest.json` mit Version, Build-Profil und App-Prüfsummen.
- `SHA256SUMS.txt` mit ZIP- und Manifest-Prüfsummen.

Das Release-Profil prüft die App auf Diagnosecode und begrenzt den
Paketinhalt auf die vorgesehenen Dateien. Die Datendateien werden gegen
die SHA256-Werte in `tests/baseline.json` geprüft. Verschiedene Versionen
und Build-Profile benötigen getrennte Ausgabeverzeichnisse.

Ein lokaler Build veröffentlicht keinen GitHub-Release.

## Tests ausführen

```text
py -3 tools/run_tests.py
py -3 tools/check_preview.py
py -3 tools/check_preview.py --app release-ready/v1_0_0/pc-preview/physchem_final.py
```

Die 35 automatisierten Testgruppen decken Wissenskapitel, Rechnerfunktionen,
Navigation, Dateifehler, Bilddarstellung und Release-Pakete ab. Alle 39
Abbildungen werden pixelgenau mit der Referenz verglichen.
Die Tkinter-Prüfung zeichnet jeweils 20 Bildwechsel und prüft das Beenden-Menü.
Berichte und Bildvergleiche liegen unter `testbuild/hh3_preview`.

PC-Ergebnisse sind keine Messwerte für Geräte-RAM oder Ladezeiten am ClassPad.

## Diagnosebuild

```text
py -3 tools/build_hh3.py --diagnostics
```

Die Ausgabe liegt unter `testbuild/hh3_preview`. Im Hauptmenü dieses Builds
öffnet `HH3-Bildtest` die Vergleichsmessung und einen Test mit 20 Bildwechseln.
`Ladewege vergleichen` erfasst den ursprünglichen und den optimierten Ladeweg
mit drei Durchläufen pro Bild. Die Anzeigen enthalten ersten Aufruf, Median
und Zeitanteile für Index, Dateiöffnung, Positionierung, Lesen und Zeichnen.
DEL bricht die Messung nach dem aktuellen Bild ab.

Die regulären ClassPad- und PC-Pakete enthalten dieses Diagnosemenü nicht.

## Beiträge und Fehlerberichte

- Bestehende Rechnerergebnisse und Navigation bei Änderungen erhalten.
- Eingabeprüfungen und Tests zusammen mit neuen Formeln ergänzen.
- Byteindizes und Paketkennungen bei Änderungen an Datendateien berücksichtigen.
  Die Git-Dateiregeln verhindern dafür ungewollte Zeilenenden-Konvertierungen.
- Für neue Abbildungen nur eigene oder ausdrücklich weiterverteilbare Quellen verwenden.
- Bei Gerätefehlern PythonExtra-Version, betroffenen Artikel oder Rechner,
  vollständigen Fehlertext und reproduzierbare Schritte angeben.

Technische Grundlagen und weitere Prüfschwerpunkte stehen in [ROADMAP.md](ROADMAP.md).
