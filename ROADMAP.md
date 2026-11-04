# Technik und Weiterentwicklung

Physik LK für Hollyhock 3 verwendet PythonExtra.hh3. Wissen und Abbildungen
liegen in externen Datendateien und werden bei Bedarf geladen.
Die Build- und Testanleitung steht in [DEVELOPMENT.md](DEVELOPMENT.md).

## Dateizugriff

- Verzeichnisse, Wissenskapitel und Medien werden über Bytepositionen angesprochen.
- `seek()` positioniert den Dateizeiger; `tell()` prüft die erreichte Position.
- Bei fehlender oder unbrauchbarer Positionierung wird die Datei erneut
  geöffnet und kontrolliert sequenziell gelesen.
- `readinto()` liest in Blöcken bis 4 KiB. Kurze Leseergebnisse werden behandelt;
  Paketkennungen und Bildprüfsummen dienen der Fehlererkennung.

## Speicher und Darstellung

- Es wird jeweils nur eine Abbildung geladen, ohne dauerhaften Bildcache.
- Bei erfolgreichen Bildaufrufen gibt es keine laufenden `gc.mem_free()`-Abfragen.
- Erzwungene Speicherbereinigungen erfolgen vor der Bildpuffer-Allokation
  und nach der Pufferfreigabe; die automatische Bereinigung bleibt eingeschaltet.
- Rasterbilder verwenden rohe RGB565-Paletten und vollständige
  128-Zeilen-Zeichenabschnitte. Vektorskizzen bestehen aus Text- und Linienbefehlen.

## Testgrundlage

Die automatisierten Tests verwenden eine eingefrorene HH2-Referenz und
feste SHA256-Werte für die Datendateien. Rechnerfunktionen, Wissenskapitel,
Bilddarstellung und Navigation werden gegen Referenzwerte geprüft.
Zusätzliche Tests behandeln Dateifehler, kurze Lesezugriffe, eingeschränkte
Bytearray-Funktionen, fehlgeschlagene Positionierung und die Release-Pakete.

Die Geräteerprobung bezieht sich auf MicroPython
`v1.25.0-preview.602.gfca9b039f` vom 17.02.2026, sh7305 mit sh-4a.
Ein vollständiger Hardwaretest aller Funktionen und standardisierte
Ladezeit- oder RAM-Messungen liegen nicht vor.

## Weitere Prüfschwerpunkte

- Ladezeiten auf dem ClassPad nach Indexzugriff, Positionierung, Lesen,
  Prüfsumme, Zeichnen und Bildschirmaktualisierung aufschlüsseln.
- Ersten Aufruf und Median mehrerer Aufrufe für Vektor-, P4- und P8-Bilder vergleichen.
- Speicherentwicklung bei wiederholten Bildwechseln auf dem Gerät erfassen.
- Kompatibilität mit weiteren HH3-/PythonExtra-Versionen prüfen und dokumentieren.

Für reproduzierbare Messungen enthält der optionale Diagnosebuild den
ursprünglichen und den optimierten Ladeweg. Beide werden unter derselben
Laufzeit mit denselben Bildern verglichen. PC-Messwerte und Gerätewerte
sind getrennt auszuwerten.

## Technische Quellen

- [PythonExtra: Änderungen zwischen HH2- und HH3-Build](https://github.com/TheRainbowPhoenix/PythonExtra/compare/d5d03e81b...fca9b039f)
- [PythonExtra: Einrichtung des Python-Heaps](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/ports/sh/main.c)
- [MicroPython: gc.mem_free() und gc_info()](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/py/modgc.c)
- [MicroPython: Heap-Verwaltung](https://github.com/TheRainbowPhoenix/PythonExtra/blob/fca9b039f/py/gc.c)
