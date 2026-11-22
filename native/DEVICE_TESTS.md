# Geraetestand

## Beta.6: Hauptmenue vereinfacht, Geraetetest offen

Paket: `physik_hh3_native_v2_0_0_beta_6_test.zip`. Alle drei Dateien gemeinsam
uebertragen. Wissens- und Bilddatei sind gegenueber Beta.5 unveraendert.

- Hauptmenue mit genau fuenf Punkten: Theorie, Formeln nach Themen, Formeln A-Z,
  Physik-Rechner und Beenden. **Formeln nach Groessen** darf nicht mehr erscheinen.
- Alle fuenf Eintraege mit Touch und Tasten erreichen; beim Zurueckgehen die
  richtige Hauptmenueauswahl wiederherstellen.
- Themensuche: 09 Schwingungen oeffnet fachliche Unterthemen, keine Groessenliste.
- A-Z: Buchstabe und Groesse waehlen, Formel oeffnen, mehrstufig zurueckgehen.
- Theorie mit Bildansicht und erhaltener Textposition pruefen.
- Physik-Rechner: T aus 2 Hz ergibt 0.5 s; danach Beenden zum Launcher.

Die bisherigen offenen Stichproben gelten weiter. Softwaretests und der
HH3-Cross-Build ersetzen keinen vollstaendigen Geraetetest.

## Beta.5: Alphabetische Suche, Geraetetest offen

Alle drei Dateien aus dem Beta.5-Paket gemeinsam ersetzen.
Die alphabetische Suche ist noch nicht am ClassPad bestaetigt.

- Hauptmenue: Theorie, Formeln nach Groessen, Formeln nach Themen, Formeln A-Z,
  Physik-Rechner und Beenden. Alle sechs Eintraege mit Touch und Tasten oeffnen.
- **Formeln A-Z > A**: Beschleunigung, Flaeche, Leiterabstand und
  Schwingungsamplitude als getrennte Groessen erreichen.
- Unter D die Delta-Groessen, unter O die Kreisfrequenz finden.
- Unter Q Ladung, Blindleistung und Schwingkreisguete unterscheiden; unter R
  die verschiedenen rho-Groessen pruefen.
- Eine lange Buchstaben-/Groessenliste scrollen, Formel oeffnen und mehrstufig
  zurueckgehen. Auswahl und Listenposition sollen auf jeder Ebene erhalten bleiben.
- Unter L den Artikel **De Broglie: klassisch und relativistisch** ueber eine
  passende Wellenlaengen-Groesse oeffnen; Bilder wechseln und zur Textposition zurueckkehren.
- Nach alphabetischer Suche einen bisherigen Suchweg, einen Rechner und Beenden pruefen.
- In einer separaten Testkopie Beta.4-`wissen.txt` verwenden: Paketfehler statt
  Absturz; Zurueck ins Hauptmenue und Rechner bleiben bedienbar. Danach Beta.5 wiederherstellen.

Automatische Abdeckung: alle 142 Groessen, 368 Formelkapitel und 32 Ergaenzungsartikel;
jede Zuordnung, Sortierung, keine leeren Buchstaben, unveraenderte Unterrichtssuchwege,
Ruecknavigation, Touch und Bildrueckkehr. Softwaretests sind keine Hardwarefreigabe.

## Beta.4: Unterrichtsnavigation, Geraetetest offen

Alle drei Dateien aus dem Beta.4-Paket gemeinsam ersetzen. PC-Tests sind keine
Geraetefreigabe; der Programmbereich bleibt auf 337.920 Bytes begrenzt.
Das ist kein Messwert fuer den freien Arbeits-RAM.

- In Theorie und beiden Formelsuchen die 15 Oberthemen in Unterrichtsreihenfolge pruefen.
- Lange Listen mit Touch und Tasten durchblaettern; mehrstufig zurueckgehen.
  Auswahl und Scrollposition muessen auf jeder Ebene erhalten bleiben.
- Theorie: 11 Quantenphysik, Materiewellen, De-Broglie-Artikel; Text scrollen,
  Bilder oeffnen, 20-mal wechseln und zur gleichen Textposition zurueckkehren.
- Groessensuche: 09 Schwingungen, Leistung; die Verlustleistungsformel pruefen.
- Lorentzkraft unter 05 und 06, Wechselstrom unter 07 und 09 sowie Potentialtopf
  unter 11 und 12 finden. Gleichnamige Symbole mit verschiedenen Bedeutungen pruefen.
- In Abiturvorbereitung Wellen-/Quanten-/Atomthemen, unter Sonstiges Uebersichten,
  Konstanten und gemischte Feldbezuege oeffnen.
- Einen alten Rechner (T aus 2 Hz), eine neue Maske und Beenden pruefen.
- In einer separaten Testkopie `wissen.txt` aus Beta.3 verwenden: neue Navigation
  meldet einen Paketfehler, ohne abzustuerzen; der Rechner bleibt bedienbar.
  Danach alle drei Beta.4-Dateien wiederherstellen.
- Ladezeit der Oberthemen und langer Listen sowie freien RAM am Geraet protokollieren.

Die Beta.3-Stichproben unten bleiben weiterhin offen. Automatische Berichte
stehen nach dem Build in `dist/navigation-tests.log`, `dist/compact-navigation-tests.log`
und `dist/navigation-data-tests.log`. Beta.3 ist unveraendert gesichert.

## Beta.3: Erweiterter lokaler Testbuild, noch offen

Keine Geraetefreigabe fuer beta.3. PC- und SH4-Buildpruefungen ersetzen den
ClassPad-Test nicht. Die Programmbereichsgroesse ist kein freier Heap-RAM-Wert.

Vor dem Test alle drei Dateien aus dem beta.3-Paket gemeinsam ersetzen.

- Hauptmenue mit fuenf Punkten, Beenden zum Launcher.
- Neue Kategorien am Ende des Physik-Rechners ueber Touch und Tasten erreichen.
- Wurf: 100 km/h, 30 Grad, gleiche Hoehe, g=9.81: Reichweite etwa 68.1172 m.
- Bremsen: 80 km/h, 1 s, mu=0.5: Anhalteweg etwa 72.5614 m; Hindernis 9 m in Reaktionsphase.
- Kondensator: 50 uF, 4000 V: 0.2 C und 400 J; mit 100 Ohm tau=5 ms.
- Elektronvergleich bei 50 kV: 5.48474 pm klassisch und 5.35531 pm relativistisch.
- Planck-Fit mit zwei und mehr Punkten, Grenzwellenlaenge und Diagramm.
- Messpunktzahl erhoehen/verkleinern, zurueckgehen, Einheiten korrigieren, erneut rechnen.
- Eine 32-Punkte-Millikan-Reihe bis zur letzten Zeile eingeben und bearbeiten.
- Grafikcursor mit Pfeilen und Touch, Ansichtswechsel, Tabelle und Scrollen.
- Achsen aendern, gleiche/vertauschte Grenzen abweisen, Abbruch und Rueckkehr.
- Nach Achsenbearbeitung mit den urspruenglichen Eingaben erneut rechnen.
- Drei Simulationen mit Vorgaben starten, Schrittweite aendern, Ereignisse ablesen.
- 20 Grafikwechsel; danach altes Bild, alten Rechner und Beenden pruefen.
- Neue Theorie ueber alle drei Suchwege, einschliesslich Artikel mit alten Bildern.
- In einer separaten Testkopie alte `wissen.txt` verwenden: neue Artikel und Rechner
  melden Paketfehler; alte Theorie bleibt lesbar. Danach vollstaendiges Paket wiederherstellen.

Protokoll: Datum, HH3-Version, Parameter/Einheiten, Bedienweg, Ergebnis, gemessene
Dauer und gegebenenfalls RAM. Beobachtete Fehler nicht als bestanden markieren.

## Alpha.2: Rueckmeldung vom 8. November 2026

Auf dem ClassPad mit Hollyhock 3 bestaetigt:

- `wissen.txt` und `bilder.dat` werden gemeinsam mit `physik.hh3` im
  Hauptverzeichnis gefunden.
- Bilder laden schnell und vollstaendig; Wechsel wirken fluessig.
- Die Navigation laesst sich gut bedienen.

Das ist eine qualitative Rueckmeldung, keine gemessene Zeitreihe oder Pruefung
saemtlicher Funktionen. Die damalige Version enthielt nur drei Theorie-Themen.

## Alpha.3: Noch nicht am Geraet bestaetigt

Diese Version ergaenzt alle Theorie-/Formelmenues und den nativen Rechner.
Automatische Pruefungen und Quell-/Paketpruefsummen stehen nach dem Build
in `dist/manifest.json`, `dist/host-tests.log`, `dist/calculator-tests.log`,
`dist/sdk-file-tests.log` und `dist/SHA256SUMS.txt`.

Offene Geraetestichproben:

- Beide Formelsuchen, lange Menues und Ruecknavigation.
- Touch- und Hardware-Zahleneingabe, `E`, DEL, Einheiten und mehrere Eingabefelder.
- Periodendauer aus 2 Hz, Photonenergie aus 500 nm und ein Planck-Fit.
- Fehlermeldung bei unzulaessigen Eingaben, danach Korrektur und Neuberechnung.
- Bildwechsel, erhaltene Textposition und Rueckkehr zum Launcher.

Die Rueckmeldung zu alpha.2 ersetzt diese Pruefung nicht.

## Beta.1

`2.0.0-beta.1` veroeffentlicht den Funktionsstand von alpha.3 mit angepasster
Versionsanzeige. Rechenlogik, Dateileser und Datendateien sind unveraendert.
Die oben aufgefuehrten offenen Geraetestichproben gelten weiterhin.

## Beta.2: Lokaler Testbuild, 20. November 2026

Paket 1A ergaenzt sieben Masken fuer gleichmaessig beschleunigte Bewegung,
passende Einheiten und Mehrfachergebnisse. Die bisherigen 167 Funktionen,
Wissens-/Mediendateien und Dateiladekorrekturen bleiben erhalten.
Dieser Stand ist nicht veroeffentlicht und noch nicht auf dem ClassPad bestaetigt.

PC-Pruefungen: alte Referenztests plus eigene Original-/Umkehr-/Grenzfalltests,
Menue-, Touch-/Tasten- und Mehrfachergebnisnavigation, Sanitizer und echter
SH4-Cross-Build. Ergebnisse stehen nach erfolgreichem Build in `dist`.

Offen am ClassPad: Bob-Verlauf, beide Zeitwurzeln, unmoeglicher Zielort und
Korrektur, Einheitenwechsel km/h/m/s sowie km/m und h/s, wissenschaftliche
Eingabe, Ergebnis-Scrollen und wiederholte Berechnung; anschliessend Bilder,
alte Rechner und Beenden stichprobenartig pruefen. Eingaben stehen in `README.md`.
PC-Speicherpruefungen oder die ELF-Programmgroesse sind keine Messung des freien
Geraete-Arbeits-RAMs und keine Geraete-Laufzeitmessung.
