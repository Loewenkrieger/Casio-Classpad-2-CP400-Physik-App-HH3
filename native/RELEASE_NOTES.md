# Physik LK Native v2.0.0-beta.1

Erste oeffentliche native Beta fuer den Casio ClassPad II (fx-CP400) mit
Hollyhock 3. Die App startet direkt als `physik.hh3`, ohne PythonExtra.

## Enthalten

- 146 Theorieartikel und 368 eindeutige Formeln, erreichbar ueber 840
  Menueeintraege und insgesamt 514 einzeln geladene Wissenskapitel.
- 17 Rechnerkategorien mit 163 festen Formeln und vier Planck-Messreihen.
- Einheitenwahl, wissenschaftliche Schreibweise und Touch-Zahlentastatur.
- Alle 39 oeffentlichen Vektorskizzen und Rasterbilder, mit Rueckkehr zur
  bisherigen Textposition.
- Hauptmenue mit Theorie, beiden Formelsuchen, Physik-Rechner und Beenden.

Die Inhalte entsprechen der bisherigen Python-Version. Private PDF-Ausschnitte
sind nicht enthalten. Rechenlogik und Datendateien sind gegenueber dem lokalen
alpha.3-Stand unveraendert; Versionsanzeige und Paketierung tragen jetzt Beta.1.

## Installation

1. `physik_hh3_native_v2_0_0_beta_1.zip` herunterladen und entpacken.
2. `physik.hh3`, `wissen.txt` und `bilder.dat` gemeinsam ins Hauptverzeichnis
   des ClassPad neben `run.bin` kopieren.
3. Im Hollyhock-3-Launcher **Physik LK Native** starten.

Es sind keine zusaetzlichen Python-Dateien erforderlich. Die ZIP enthaelt genau
die drei App-Dateien. `SHA256SUMS.txt` prueft ZIP und `manifest.json`; das Manifest
enthaelt zudem die Einzelpruefsummen und die automatischen Testergebnisse.

## Beta-Status

**Vorabversion, kein stabiler Release.** Die native Basis fuer Dateizugriff,
vollstaendige schnelle Bilder und Navigation wurde am ClassPad bestaetigt.
Eine vollstaendige Hardwarepruefung aller Menues und Rechnerfunktionen steht
noch aus. Es werden keine gemessenen Ladezeiten oder allgemeine
Leistungsgarantien angegeben.

Automatische Tests pruefen alle Kapitel, Medien, 840 Artikelverweise,
167 Rechnerformulare, 787 numerische Rechenfaelle, sieben Referenz-Fits und alle
vier Messreihen-Ablaeufe. Speicherpruefungen nutzen AddressSanitizer,
UndefinedBehaviorSanitizer und LeakSanitizer. Die drei Rasterbilder werden
pixelgenau mit der Python-Referenz verglichen; 20 Bildwechsel sind abgedeckt.

Die bestehende Python-Version v1.0.0 bleibt unveraendert verfuegbar.
