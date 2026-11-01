# Physik LK für den Casio ClassPad II mit Hollyhock 3

Die eigenständige HH3-Projektlinie der Physik-Lernapp für den ClassPad II.
Der Fokus liegt auf **Hollyhock 3**, **PythonExtra.hh3** und einem daran
angepassten Ladeweg für Wissen und Abbildungen.

> [!NOTE]
> **HH3-Release v1.0.0.** Die App enthält keine Grafiktests, Vergleichsladewege
> oder Diagnosemenüs. Der optimierte Ladeweg und alle fachlichen Abbildungen
> bleiben erhalten. Die Funktion wurde vom Nutzer am ClassPad bestätigt;
> genaue Ladezeiten und ein vollständiger Hardwaretest aller Funktionen
> liegen noch nicht vor. PythonExtra wird weiterhin benötigt.

## Downloads

| Paket | Verwendung |
| --- | --- |
| [ClassPad v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_classpad_v1_0_0.zip) | Genau drei App-Dateien für Hollyhock 3 / PythonExtra.hh3 |
| [PC-Vorschau v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/physik_hh3_pc_v1_0_0.zip) | Gleiche App mit Grafiksimulation und Startdatei für Windows |
| [Prüfsummen](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v1.0.0/SHA256SUMS.txt) | SHA256 der ZIPs und des Paketmanifests |

Alle Downloads stehen im [HH3-Release v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/tag/v1.0.0).
Das Repository bleibt privat; für den Download ist ein berechtigtes GitHub-Konto nötig.

## Funktionen

- **146 Theorieartikel**, **368 eindeutige Formeln** und **514 Wissenskapitel**.
- **167 Rechner- und Messreihenfunktionen** mit passenden Einheiten.
- Mechanik, Schwingungen, Elektrizität und Wechselstrom sowie Quanten- und Atomphysik.
- **39 eigene fachliche Abbildungen** im übernommenen öffentlichen Medienpaket.
- Wissen und Bilder werden nach Bedarf geladen.

Der fachliche Umfang bleibt erhalten. Die Optimierung betrifft
Ladegeschwindigkeit und Speicherverwaltung, nicht Bildqualität
oder Formelinhalte. Arbeitsstand und weitere Prüfungen stehen in [ROADMAP.md](ROADMAP.md).

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

Die PC-Vorschau enthält dieselbe App ohne Grafiktest-Menü. `gint.py` simuliert
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

Die HH2-Ausgangsbasis liest auf dem Weg zum Bild viele kleine Dateiblöcke,
führt häufig Speicherbereinigungen durch und fragt während der Bilddiagnose
mehrfach den freien RAM ab. Die HH3-Version springt nach bestätigtem
`seek()`/`tell()` direkt zur Position und liest mit `readinto()` bis zu 4 KiB
pro Aufruf. Bei unbrauchbarer Positionierung wird die Datei neu geöffnet
und kontrolliert sequenziell gelesen.

Die unveränderte HH2-Ausgangsbasis zeigte unter dem gemeldeten HH3-Build
stark verlängerte Bildladezeiten. Nach der Optimierung bestätigt der Nutzer
die Funktion am Gerät und berichtet sogar eine subjektiv schnellere
Bildanzeige als beim bisherigen HH2-Build. Getrennte Gerätezeiten für die
einzelnen Ladeschritte und ein gemessener Beschleunigungsfaktor liegen noch
nicht vor. Der Vergleichsbuild stellt beide Ladewege für reproduzierbare
Messungen unter derselben HH3-Laufzeit bereit.

Am PC gezählte Vorgänge bei einem kalten Bildindex, keine Gerätezeiten:

| Beugungsringe | Ursprünglich | HH3-Version |
| --- | --- | --- |
| Leseaufrufe insgesamt | 622 | 63 |
| Erzwungene Bereinigungen | 26 | 2 |
| Laufende RAM-Abfragen | 14 | 0 |

Die Bilddaten, RGB565-Paletten und vollständigen 128-Zeilen-Zeichenabschnitte
wurden nicht verändert. Paketprüfung und Prüfsummen bleiben aktiv.

## Prüfstatus und Herkunft

Ausgangspunkt ist ausschließlich die öffentliche App aus dem
[HH2-Release v1.0.0](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App/releases/tag/v1.0.0).
Wissen, Rechner und Bilddaten bleiben unverändert. Eine eingefrorene
Referenzdatei und feste SHA256-Werte sichern den Vergleich gegen den
öffentlichen Ausgangsstand; die Tests vergleichen nicht die neue Version mit sich selbst.

Der optimierte HH3-Build wurde auf dem ClassPad des Nutzers erfolgreich
erprobt; die Rückmeldung bestätigt eine subjektive Beschleunigung gegenüber
dem bisherigen HH2-Build. Das ist **kein vollständiger HH3-Hardwaretest**
aller Funktionen und keine quantifizierte Leistungsmessung. Die 35
Software-Testgruppen prüfen unter anderem
alle 514 Kapitel, die Rechnerregressionen, 39 pixelgleiche Bildansichten,
kurze Lesezugriffe, defekte Dateien, eingeschränkte Bytearray-APIs,
Ersatzwege und die Rückkehr aus der Bildansicht. Beide PC-Varianten wurden
zusätzlich mit jeweils 20 vollständig gezeichneten Bildwechseln und dem
Beenden-Menü geprüft. Die ursprünglichen 48 HH2-Referenztestgruppen wurden
separat erneut ausgeführt; sie werden nicht als neue HH3-Gerätetests ausgegeben.
Vier zusätzliche Release-Prüfungen sichern unter anderem die Abwesenheit
von eingebautem Diagnosecode, ZIP-Inhalte, Versionstrennung und Prüfsummen.

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

Gemeldete HH3-PythonExtra-Version:

```text
MicroPython v1.25.0-preview.602.gfca9b039f on 2026-02-17
sh7305 with sh-4a
```

Dieses Repository beginnt mit einer eigenen Git-Historie. Die Herkunft
bleibt über das öffentliche HH2-Projekt dokumentiert; dessen Release-Tags
werden hier nicht als neue HH3-Releases übernommen.

## Hinweise

- Dieses Repository ist privat. Es enthält den optimierten HH3-Code,
  Testwerkzeuge und ausschließlich Daten aus dem bisherigen öffentlichen App-Stand.
- Private PDF-Ausschnitte, Unterrichtsmaterialien, alte ZIPs und lokale Backups
  wurden nicht übernommen. Das Medienpaket enthält nur die 39 eigenen Abbildungen.
- Weitere Geräteprüfungen betreffen genaue Ladezeiten und die vollständige
  Funktionsabdeckung. Die Veröffentlichung erweitert den bestätigten
  Hardware-Prüfumfang nicht automatisch.
- Dieses Community-Projekt ist nicht mit Casio verbunden. Schul- und
  Prüfungsregeln beachten; keine Zusicherung für den Prüfungsmodus.
- Installationsquellen zuletzt am **28. Oktober 2026** geprüft.
