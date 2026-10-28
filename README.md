# Physik LK für den Casio ClassPad II mit Hollyhock 3

Die eigenständige HH3-Projektlinie der Physik-Lernapp für den ClassPad II.
Der Fokus liegt auf **Hollyhock 3**, **PythonExtra.hh3** und einem daran
angepassten Ladeweg für Wissen und Abbildungen.

> [!WARNING]
> **Noch nicht optimierter Entwicklungsstand.** Die drei App-Dateien sind
> zunächst bytegleich aus dem öffentlichen HH2-Release v1.0.0 übernommen.
> Unter der gemeldeten HH3-/PythonExtra-Version laden Bilder deutlich
> langsamer. Dieses Repository enthält noch keinen stabilen HH3-Release
> und keine bereits umgesetzte Ladeoptimierung.

## Funktionen der Ausgangsbasis

- **146 Theorieartikel**, **368 eindeutige Formeln** und **514 Wissenskapitel**.
- **167 Rechner- und Messreihenfunktionen** mit passenden Einheiten.
- Mechanik, Schwingungen, Elektrizität und Wechselstrom sowie Quanten- und Atomphysik.
- **39 eigene fachliche Abbildungen** im übernommenen öffentlichen Medienpaket.
- Wissen und Bilder werden nach Bedarf geladen.

Der fachliche Umfang bleibt erhalten. Die geplanten Änderungen betreffen
zunächst Ladegeschwindigkeit und Speicherverwaltung, nicht Bildqualität
oder Formelinhalte. Der genaue Arbeitsplan steht in [ROADMAP.md](ROADMAP.md).

## Voraussetzungen

- Ein ClassPad II mit funktionierendem Hollyhock 3.
- Die dazu passende PythonExtra-Datei **`PythonExtra.hh3`**.
- Zugriff auf dieses private Repository.

Hollyhock und PythonExtra werden nicht mit dieser App mitgeliefert.
Diese Projektlinie strebt keine gemeinsame HH2-/HH3-Laufzeitfreigabe an.
Der bisherige HH2-Stand bleibt im
[öffentlichen Ursprungsprojekt](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App)
erhalten.

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

## 2. Entwicklungsstand auf dem Gerät testen

1. Verwende die drei zusammengehörigen App-Dateien aus diesem Repository.
2. Lege sie gemeinsam in einen Ordner wie `physik_hh3` auf dem ClassPad.
3. Wirf das Gerät sicher aus und starte PythonExtra neu.
4. Wähle `physchem_final.py` in der Dateiliste aus.

> [!IMPORTANT]
> Verwende im gesamten Startpfad keine Leerzeichen, Bindestriche oder Klammern.
> Der automatisch entpackte GitHub-Repository-Ordner enthält Bindestriche:
> Starte die App deshalb nicht direkt daraus. Unterstriche wie in
> `physik_hh3` sind geeignet. Alle drei Dateien immer gemeinsam austauschen.

Es gibt hier noch keine fertigen HH3-Download-ZIPs. README, Roadmap und
Git-Konfigurationsdateien werden auf dem Taschenrechner nicht benötigt.
Eine PC-Vorschau und Entwicklungswerkzeuge sind in diesem Projektstart
ebenfalls noch nicht enthalten.

## Bedienung

Das Hauptmenü der übernommenen App enthält:

- Theorie
- Formeln nach Groessen
- Formeln nach Themen
- Physik-Rechner
- Beenden

Touch und Pfeiltasten dienen der Auswahl und Navigation. DEL beziehungsweise
Zurück führt zur vorherigen Ansicht. In Artikeln mit Bildern öffnet
`Abbildungen (EXE)` oder die EXE-Taste den Betrachter. Links/rechts wechselt
die Darstellung; Zurück stellt die bisherige Textposition wieder her.

## Bekanntes Leistungsproblem

Die Ausgangsbasis liest auf dem Weg zum Bild weiterhin viele kleine
Dateiblöcke, führt häufig Speicherbereinigungen durch und fragt während
der Bilddiagnose mehrfach den freien RAM ab. Diese Maßnahmen stammen aus
der bisherigen HH2-Entwicklung.

Unter dem gemeldeten HH3-Build werden stark verlängerte Bildladezeiten
beobachtet. Der vergrößerte Python-Heap und die bisherigen Ladewege sind
konkrete Ansatzpunkte, aber die tatsächlichen Zeitanteile wurden auf dem
Gerät noch nicht getrennt gemessen. Die [Roadmap](ROADMAP.md) sieht deshalb
einen Vergleich zwischen unverändertem und optimiertem Ladeweg vor.

## Prüfstatus und Herkunft

Ausgangspunkt ist ausschließlich die öffentliche App aus dem
[HH2-Release v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App/releases/tag/v1.0.0).
Wissen, Rechner und Bilddaten wurden für diesen Projektstart nicht verändert.
Die Startdateien werden bytegenau gegen diesen öffentlichen Stand geprüft.

Die bisherigen Softwaretests und HH2-Gerätebeobachtungen sind **keine
HH3-Freigabe**. Ein vollständiger Funktionstest und Leistungsnachweis für
Hollyhock 3 stehen noch aus.

Gemeldete HH3-PythonExtra-Version:

```text
MicroPython v1.25.0-preview.602.gfca9b039f on 2026-02-17
sh7305 with sh-4a
```

Dieses Repository beginnt mit einer eigenen Git-Historie. Die Herkunft
bleibt über das öffentliche HH2-Projekt dokumentiert; dessen Release-Tags
werden hier nicht als neue HH3-Releases übernommen.

## Hinweise

- Dieses Repository ist privat und enthält nur den bisherigen öffentlichen App-Stand.
- Private PDF-Ausschnitte, Unterrichtsmaterialien, alte ZIPs und lokale Backups
  wurden nicht übernommen. Das Medienpaket enthält nur die 39 eigenen Abbildungen.
- Die Trennung allein beschleunigt noch nichts. Die Ladeoptimierung ist der
  nächste, gesonderte Umsetzungsschritt.
- Dieses Community-Projekt ist nicht mit Casio verbunden. Schul- und
  Prüfungsregeln beachten; keine Zusicherung für den Prüfungsmodus.
- Installationsquellen zuletzt am **28. Oktober 2026** geprüft.
