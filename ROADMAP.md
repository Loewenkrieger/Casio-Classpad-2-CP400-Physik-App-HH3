# HH3-Ladeoptimierung

Status: **HH3-Release v1.0.0, softwareseitig geprüft und am Gerät erprobt**.
Der neue Ladeweg, ein getrennter Vergleichsbuild und selbstständige
Regressionstests sind umgesetzt. Wissen und Bilddaten stimmen weiterhin
bytegenau mit der öffentlichen HH2-Ausgangsbasis überein.
Der Nutzer bestätigt die Funktion auf seinem ClassPad und berichtet subjektiv
kürzere Bildladezeiten als beim bisherigen HH2-Build. Genaue Gerätezeiten und
ein vollständiger Hardwaretest aller Funktionen stehen weiterhin aus.

## Ziel und Grenzen

- Gezielt Hollyhock 3 und PythonExtra.hh3 unterstützen, zunächst mit dem
  gemeldeten MicroPython-Build v1.25.0-preview.602.gfca9b039f vom 17.02.2026.
- Bilder schneller laden, ohne Auflösung, Farben oder fachliche Inhalte zu reduzieren.
- Weiterhin nur `physchem_final.py`, `wissen.txt` und `bilder.dat` auf dem Gerät.
- Kein zusätzlicher Bildcache und keine dauerhafte Abschaltung der automatischen
  Speicherbereinigung in dieser ersten Optimierung.
- Der HH2-Release und seine Download-ZIPs bleiben unverändert im Ursprungsprojekt.

## 1. Getrennten Testbuild aufbauen

Umgesetzt: eingefrorene öffentliche Referenz, eigener Builder, Tests und
PC-Simulation. Bewusst nur benötigte öffentliche Teile übernommen, nicht
den gesamten alten Builder oder private PDF-Werkzeuge.

- Benötigte öffentliche Entwicklungsquellen, Build-Werkzeuge und Regressionstests
  gezielt aus der bisherigen lokalen Entwicklung übernehmen. Keine komplette
  Workspace-Kopie und keine privaten PDF-Materialien einchecken.
- Einen ausdrücklichen HH3-Testmodus im Builder vorsehen. Ausgabe getrennt vom
  Startstand unter `testbuild/hh3_preview` und mit importtauglichen ZIP-Ordnernamen.
- `wissen.txt` und `bilder.dat` zunächst bytegenau aus dieser Ausgangsbasis
  beibehalten. Nur die Python-App und die Testwerkzeuge ändern.
- Die private Sichtbarkeit des Repositories ist keine Erlaubnis, zusätzliche
  private Unterrichtsmedien zu übernehmen. Dafür wäre eine eigene Freigabe nötig.

## 2. Dateizugriff und Speicherverwaltung optimieren

Umgesetzt und softwareseitig geprüft; Funktion und subjektive Beschleunigung
sind durch die Geräteerprobung des Nutzers bestätigt.

- Mit `seek()` direkt zu Datensätzen und Bilddaten springen. Auch gemeinsam
  genutzte Verzeichniszugriffe einbeziehen, damit der Bildindex nicht weiter
  über den alten sequenziellen Umweg gelesen wird.
- Die erreichte Position mit `tell()` prüfen. Bei fehlender oder unbrauchbarer
  Positionierung erneut öffnen und kontrolliert sequenziell lesen.
- Im HH3-Ladeweg 4-KiB-Blöcke statt 512-Byte-Blöcken verwenden. `readinto()`,
  kurze Leseergebnisse, Paketkennungen und Bildprüfsummen weiterhin korrekt behandeln.
- Bei erfolgreichen Bildaufrufen keine fortlaufenden `gc.mem_free()`-Abfragen.
  Bei Fehlern weiterhin Schritt, Fehlertyp und erst dann ermittelten freien RAM anzeigen.
- Keine erzwungene Bereinigung nach jeweils 8 KiB. Gezielt vor der großen
  Bildpuffer-Allokation und nach der Pufferfreigabe bereinigen.
- Die automatische Speicherbereinigung eingeschaltet lassen.
- Rohe RGB565-Paletten, vollständige 128-Zeilen-Zeichenabschnitte,
  Bildbeschriftungen und Bedienung erhalten.

## 3. Auf dem ClassPad vergleichen

Der Vergleich ist im Diagnosebuild implementiert. Eine positive qualitative
Geräterückmeldung liegt vor; genaue Messwerte fehlen noch.

Nur der Testbuild erhält den Menüpunkt `HH3-Bildtest`. Der bisherige und der
optimierte Ladeweg müssen mit derselben Laufzeit und denselben Bildern
verglichen werden.

- Elektronenbeugungsröhre, Beugungsringe und letztes Bild im Paket verwenden.
- Je drei Durchläufe pro Bild und Ladeweg; ersten Aufruf und Median getrennt ausweisen.
- Verzeichniszugriff, Positionierung, Lesen, Prüfsumme, Zeichnen und
  Bildschirmaktualisierung getrennt messen.
- RAM nur außerhalb der Zeitmessungen erfassen. Ergebnisse am Bildschirm
  anzeigen, ohne zusätzliche Gerätedatei.
- Anschließend 20 Bildwechsel auf vollständige Darstellung und Pufferfreigabe prüfen.
- PC-Zahlen ausdrücklich von echten Gerätezeiten und Geräte-RAM-Werten unterscheiden.

## 4. Softwareprüfung und Abnahme

- Die bisherigen 48 Testgruppen als Referenz erhalten und mit öffentlichen
  Daten prüfen; nicht gegen frisch erzeugte Ergebnisse selbst vergleichen.
  Die vollständigen ursprünglichen 48 Gruppen bleiben unverändert im HH2-Projekt.
  Im HH3-Projekt werden passende wissenschaftliche und Bedienungstests
  übernommen und durch gezielte Tests des neuen Ladewegs ergänzt; private
  PDF- und alte Produktionsprofil-Tests werden nicht als neue HH3-Tests ausgegeben.
- HH3-Tests für erfolgreiche und fehlerhafte Positionierung, Ersatzweg, kurze
  Lesezugriffe, fehlende beziehungsweise beschädigte Dateien und Pufferfreigabe ergänzen.
- Alle Bilder pixelgenau mit der Ausgangsbasis vergleichen. Sämtliche
  Wissenskapitel, Rechnerfunktionen und die Navigation auf Regressionen prüfen.
- Abnahmeziel: weniger Dateioperationen, keine laufenden RAM-Abfragen und
  messbar kürzere Ladezeiten der bisher langsamen Rasterbilder ohne Funktionsverlust.
- Keine konkrete Beschleunigung vor dem Gerätetest zusagen. Bei verbleibenden
  Engpässen anhand der getrennten Messwerte weiterarbeiten.

## 5. Veröffentlichung v1.0.0

Der Nutzer hat nach erfolgreicher Geräteerprobung die Veröffentlichung
des aktuellen PythonExtra-Stands ohne Grafiktests in der App freigegeben.
Der Produktionsbuild enthält ausschließlich die fünf regulären Menüpunkte;
Diagnosefunktionen, Vergleichsladewege und Testbilder sind nicht enthalten.
Die Software-Tests bleiben als separate Entwicklungswerkzeuge erhalten.

Die ClassPad- und PC-Pakete werden mit Manifest und SHA256-Prüfsummen
bereitgestellt. Das Repository bleibt privat; der HH2-Release bleibt unverändert.
Genaue Gerätezeiten und ein vollständiger Hardwaretest stehen weiterhin aus.
Eine native C-/C++-Portierung ist nicht Bestandteil dieses Releases.

## Technische Quellen

- [PythonExtra: Änderungen zwischen den gemeldeten alten und neuen Builds](https://github.com/TheRainbowPhoenix/PythonExtra/compare/d5d03e81b...fca9b039f)
- [HH3-PythonExtra: Einrichtung des Python-Heaps](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/ports/sh/main.c)
- [MicroPython: gc.mem_free() verwendet gc_info()](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/py/modgc.c)
- [MicroPython: gc_info() durchläuft die Heap-Verwaltung](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/py/gc.c)
