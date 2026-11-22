# Physik LK Native v2.0.0-beta.6

Native HH3-Vorabversion. Aenderungen seit der oeffentlichen Beta.1:

- 428 Rechnerfunktionen in 50 Kategorien.
- Diagramme, Messreihen und drei feste Simulationen.
- 546 Wissenskapitel, darunter 32 neue Ergaenzungsartikel.
- Unterrichtsthemen und A-Z-Suche mit 142 Groessen.
- Hauptmenue vereinfacht; alle bisherigen Inhalte und 39 Abbildungen erhalten.

Paket: `physik_hh3_native_v2_0_0_beta_6_test.zip`, weiterhin genau
`physik.hh3`, `wissen.txt`, `bilder.dat`.
Softwaretests und HH3-Build bestanden. Die vollstaendige Geraetepruefung bleibt
offen; siehe `DEVICE_TESTS.md`. Die bisherige Python-Version bleibt verfuegbar.

# Lokaler Testbuild v2.0.0-beta.5: Formeln A-Z

Nicht auf GitHub veroeffentlicht; Geraetepruefung weiterhin offen.

- Neuer Hauptmenuepunkt **Formeln A-Z > Buchstabe > Groesse > Formel**.
- 142 Groessen, nach Formelzeichen alphabetisch und unabhaengig von Unterrichtsthemen.
- Symbol und Bedeutung bleiben sichtbar; Gross-/Kleinschreibung fuehrt nicht
  zu getrennten Buchstabenlisten. Unterschiedliche physikalische Bedeutungen
  desselben Zeichens bleiben eigenstaendige Eintraege.
- Griechische Zeichen folgen ihrer ausgeschriebenen Schreibweise.
- Alle 368 bisherigen Formelkapitel und 32 Ergaenzungsartikel sind erreichbar.
  Fachuebergreifende Verweise werden pro Groesse zusammengefasst, nicht vervielfacht.
- Die bisherigen Suchwege, Wissens-/Formeltexte, 428 Rechnerfunktionen und
  39 Abbildungen bleiben unveraendert. Menues werden weiterhin pro Ebene geladen.

Paket: `physik_hh3_native_v2_0_0_beta_5_test.zip`, weiterhin genau
`physik.hh3`, `wissen.txt`, `bilder.dat`. Alle drei Dateien gemeinsam ersetzen.
Beta.4 bleibt unter `native-backups/2026-11-22_before_alphabet_beta5` gesichert.
Automatische Berichte stehen unter `dist`, offene Geraetetests in `DEVICE_TESTS.md`.

# Lokaler Testbuild v2.0.0-beta.4: Unterrichtsnavigation

Nicht auf GitHub veroeffentlicht; vollstaendige Geraetepruefung weiterhin offen.

- Theorie und beide Formelsuchen beginnen mit den 15 Oberthemen der Lehrerordner.
- Theorie und Themensuche: Oberthema, Unterthema, Artikel beziehungsweise Formel.
- Groessensuche: Oberthema, Symbol mit klarer Bedeutung, Formel.
- Alte und neue Eintraege sind fachlich zusammengefuehrt. Fachuebergreifende
  Inhalte haben mehrere Verweise, ohne den Artikeltext zu duplizieren.
- Alle 546 Kapitel bleiben erhalten. Die 368 bestehenden Formelkapitel sind
  vollstaendig in beiden Suchwegen vertreten, einschliesslich bisher fehlender Verweise.
  Alle 32 Ergaenzungsartikel sind in allen drei Suchwegen erreichbar.
- Neue Menues werden pro Ebene aus `wissen.txt` geladen und auf Kennung,
  Pruefsumme, gueltige Ziele und doppelte Eintraege kontrolliert.
- Zurueck behaelt die vorherige Auswahl und Listenposition bei; Rechner,
  Wissens-/Formeltexte und Abbildungen sind unveraendert.

Paket: `physik_hh3_native_v2_0_0_beta_4_test.zip`, weiterhin genau
`physik.hh3`, `wissen.txt`, `bilder.dat`. Beim Update alle drei gemeinsam ersetzen.
Beta.3 bleibt unter `native-backups/2026-11-21_before_navigation_beta4` gesichert.
Pruefberichte stehen unter `dist`; offene ClassPad-Tests in `DEVICE_TESTS.md`.

# Lokaler Testbuild v2.0.0-beta.3: LK-Erweiterung

Nicht auf GitHub veroeffentlicht; keine vollstaendige Geraetefreigabe.

- 254 neue native Rechenmasken, weiterhin alle 174 bisherigen Funktionen.
- Diagramme mit Cursor, Tabellen und einstellbaren Achsen; Messreihen und Fits,
  Millikan-Auswertung, RLC-Zeiger sowie Resonanz-/Daempfungsverlaeufe.
- Drei feste Abiturmodelle mit RK4, Ereignislokalisierung und Halbierungskontrolle.
- 32 neue Wissensartikel in allen Suchwegen; alle 514 bisherigen Kapitel
  und 39 Medien bleiben erhalten.
- Kategorienweise aus der Datendatei geladene Eingabekataloge halten den
  Programmbereich klein. Keine Python-Ausfuehrung und kein freier Formelsolver.

Beim Update alle drei Dateien aus `physik_hh3_native_v2_0_0_beta_3_test.zip`
gemeinsam uebertragen: `physik.hh3`, `wissen.txt`, `bilder.dat`.
Die vollstaendige neue Funktionsliste wird unter `dist/FUNKTIONEN.md` erzeugt.
Pruefberichte und Pruefsummen stehen unter `dist`, Hardwarestatus in `DEVICE_TESTS.md`.

# Lokaler Testbuild v2.0.0-beta.2: Paket 1A

Nicht auf GitHub veroeffentlicht. Sieben neue native Masken unter
**Physik-Rechner > Kinematik: beschleunigt**: Bewegungsverlauf, Zeit bis Ort,
Zeit bis Geschwindigkeit, Beschleunigung aus Geschwindigkeiten oder Orten,
Anfangsgeschwindigkeit aus Orten und Beschleunigung aus Geschwindigkeiten/Strecke.

Mehrere Ergebnisse mit Einheiten, saubere Trennung von Ortsaenderung und
Weglaenge, alle zukuenftigen Zeitloesungen und Meldungen fuer unmoegliche oder
nicht eindeutige Faelle. Zusaetzliche Eingabeeinheiten m/km/cm/mm, s/min/h/ms,
m/s und km/h gelten nur fuer die neuen Masken. Die alten 167 Funktionen bleiben
erhalten. `wissen.txt` und `bilder.dat` sind unveraendert.

Paket: `physik_hh3_native_v2_0_0_beta_2_test.zip`, weiterhin genau die drei
Dateien `physik.hh3`, `wissen.txt`, `bilder.dat`. Die Originalfaelle Bob,
Transrapid I, Gelbphase und Schnellzug I sind im PC-Test abgedeckt, sobald der
Build erfolgreich durchgelaufen ist. Abgeleitete Umkehrungen sind als solche
getrennt. Noch keine Geraetefreigabe; siehe `DEVICE_TESTS.md`.

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
