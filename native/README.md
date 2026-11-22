# Physik LK Native

Beta-Version `2.0.0-beta.6` fuer Hollyhock 3 mit Themensuche und alphabetischer Formelsuche.
Native C++20-App mit dem HH3-SDK und der Werkzeugbasis von Mathe Vektor.
Die App startet direkt als `physik.hh3`; PythonExtra wird nicht benoetigt.

Download: [Native Beta.6](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/tag/v2.0.0-beta.6).
Diese Vorabversion ist noch kein stabiler Release. Die bisherige Python-Version
bleibt separat als v1.0.0 verfuegbar.

## Verfügbar

- Hauptmenü, Touch-/Tastenbedienung, Scrollen und Beenden zum Launcher.
- Theorie und Formelsuche nach Themen mit den Oberthemen der Unterrichtsordner.
  Alle 546 Wissenskapitel bleiben erreichbar, darunter 368 bestehende Formelkapitel
  in beiden Formelsuchen und 32 ergaenzende Artikel in allen drei Suchwegen.
- Zusaetzlich **Formeln A-Z**: Buchstabenwahl, 142 themenuebergreifende Groessen
  und die zugehoerigen Formeln und Ergaenzungsartikel.
- Alle 39 oeffentlichen Abbildungen: Vektorskizzen sowie P4-/P8-Rasterbilder.
- Rückkehr aus der Bildansicht zur bisherigen Textposition.
- Native Leser für alle bestehenden Verzeichnisse, Kapitel und Medien.

- Die bisherigen 17 Rechnerkategorien mit 163 festen Formeln und vier Planck-Messreihen:
  je zwei Punkte oder 3-10 Punkte fuer `f / U_G` und `lambda / E_kin`.
- Touch-Zahlentastatur, wissenschaftliche Schreibweise und Einheitenwahl.
- Zusaetzlich **Kinematik: beschleunigt** mit sieben nativen Rechenmasken und
  mehreren Ergebnissen je Rechnung. Diese 174 Funktionen bleiben erhalten;
  zusammen mit den 254 neuen Masken sind es 428 Funktionen in 50 Kategorien.
- Neue Rechner fuer Fall/Wurf, Bremsen, Begegnungen, Energie, Stoesse und Kreisbewegung;
  Felder, Teilchenbahnen, RC/RL und Transformator; exakte Wellenoptik;
  Diagramme, Messreihen, Wechselstrom, Daempfung und drei numerische Modelle.
- Ergebnisdiagramme mit Cursor, Wertetabelle und einstellbaren Achsen.

Der bisherige Inhalt von `wissen.txt` bleibt als bytegleicher Anfangsbereich erhalten.
Die neue Paketdatei enthaelt dahinter einen getrennt gekennzeichneten und geprueften
Ergaenzungsbereich. `bilder.dat` bleibt bytegleich zum bisherigen oeffentlichen
Python-Build. Die bisherigen Formeln werden beim Bauen in festen C++-Code uebersetzt;
auf dem ClassPad gibt es weder Python-Ausfuehrung noch `eval()`.
Die Beschreibungen und Eingabekataloge der 254 neuen Masken werden ebenfalls
byteindiziert aus `wissen.txt` geladen, jeweils nur fuer die aktive Kategorie.
Die Unterrichtsnavigation liegt in einem eigenen geprueften Datenbereich;
es wird immer nur die gerade geoeffnete Menueebene geladen.
Die festen Berechnungen und ihre Eingabepruefungen bleiben nativer C++-Code.
Die erzeugte
`equations.inc`, die Python-Referenz und deren eingefrorene Datensaetze bleiben unveraendert.

## Auf dem ClassPad installieren

1. [ClassPad-ZIP der Beta.6](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/download/v2.0.0-beta.6/physik_hh3_native_v2_0_0_beta_6_test.zip)
   herunterladen und entpacken.
2. `physik.hh3`, `wissen.txt` und `bilder.dat` gemeinsam ins Hauptverzeichnis
   des Taschenrechners neben `run.bin` kopieren. PythonExtra ist nicht nötig.
3. Im HH3-Launcher **Physik LK Native** öffnen.
4. Unter **Theorie > 11 Quantenphysik** im Unterthema zu Materiewellen
   **De Broglie und Modellgrenzen** oeffnen, etwas scrollen und EXE druecken.
5. Elektronenbeugungsröhre und Beugungsringe mit links/rechts durchblättern.
6. Mit Zurück/Clear zum Artikel zurückkehren; die Textposition soll erhalten bleiben.
7. Unter **11 Quantenphysik** im Artikel **Doppelspalt mit Quantenobjekten** die
   Punkt-/Streifenabbildung und unter **12 Grundlagen Atomphysik** im Artikel
   **Emission, Absorption und Serien** das Linienspektrum pruefen.
   Bilder sollen vollstaendig und farblich korrekt sein.
8. Mehrfach zwischen Ansichten wechseln, danach über Beenden zum Launcher zurückkehren.

Das ZIP enthaelt genau `physik.hh3`, `wissen.txt` und `bilder.dat`.
Alle drei Dateien gehoeren gemeinsam in denselben Ordner. Auch beim Update
alle drei zusammen ersetzen: Aeltere Wissensdateien enthalten die neue Navigation nicht.
`cinput.py`, `gint.py`
und PythonExtra sind nicht erforderlich.

## Rechner bedienen

Werte einzeln eingeben und **Weiter** oder EXE druecken. Links/hoch fuehrt zum
vorherigen Feld. **Rechnen** berechnet das Ergebnis. **Einheit** wechselt z.B.
zwischen m/nm/pm oder J/eV/keV. Der Zahlenwert bleibt dabei gleich; seine
Interpretation aendert sich. `E` erlaubt Eingaben wie `6.626E-34`, **DEL** loescht.
Nach einem Ergebnis ersetzt die erste neue Ziffer den bisherigen Feldinhalt.

Bei Planck-Messreihen sind THz, nm und eV vorbelegt. Die vier bisherigen Ablaeufe
bleiben erreichbar; die neuen Auswertungen ergaenzen Grenzwellenlaenge, Diagramm,
Residuen und statistische Standardfehler. Die hinterlegten Konstanten bleiben unveraendert.

## Neue Funktionen finden

Die zusaetzlichen Kategorien stehen unter **Physik-Rechner** hinter den bisherigen
Eintraegen. Mit rechts/links wird seitenweise, mit hoch/runter zeilenweise geblaettert;
die Liste laesst sich auch mit Touch verschieben. Eine vollstaendige Liste mit
Menuebezeichnungen wird beim geprueften Build als `dist/FUNKTIONEN.md` erzeugt.

## Theorie und Formeln finden

- **Theorie > Oberthema > Unterthema > Artikel**
- **Formeln nach Themen > Oberthema > Unterthema > Formel**
- **Formeln A-Z > Buchstabe > Groesse > Formel**

Die Oberthemen folgen der Unterrichtsreihenfolge von **00 Kinematik** bis
**12 Grundlagen Atomphysik**. Hinzu kommen **Abiturvorbereitung** und **Sonstiges**.
Groessen tragen Symbol und Bedeutung, beispielsweise **U - Spannung**.
Fachuebergreifende Artikel sind an mehreren passenden Stellen verlinkt, aber nur
einmal gespeichert. Neue und bisherige Inhalte stehen gemeinsam in den Fachgruppen;
es gibt keine getrennten `Neu:`-Listen mehr. Zurueck stellt Auswahl und Listenposition
wieder her, auch nach einem Artikel oder einer Abbildung.
Annahmen und Vorzeichenkonventionen stehen bei den Ergebnissen.

**Formeln A-Z** zeigt die Groessen unabhaengig von Unterrichtsthemen.
Sortiert wird nach dem Formelzeichen ohne Vorrang fuer Gross- oder Kleinbuchstaben.
Symbol und ausgeschriebene Bedeutung bleiben sichtbar: beispielsweise
`a - Beschleunigung`, `A - Flaeche` und `A_s - Schwingungsamplitude`.
Gleichnamige Zeichen mit verschiedenen Bedeutungen bleiben getrennt.
Griechische Zeichen folgen ihrer Schreibweise: `Delta` unter D, `omega` unter O.
Es erscheinen nur Buchstaben mit Eintraegen. Mehrfachverweise aus verschiedenen
Unterrichtsthemen werden innerhalb einer Groesse zusammengefasst.

Zusammengehoerige Rechnungen liefern mehrere Groessen mit Einheiten. Nicht zulaessige
Zustaende, unerreichbare Ziele und mehrdeutige Faelle werden kenntlich gemacht.
Reibungszahlen, Materialwerte und `g` sind Modellparameter, keine automatisch
ermittelten Messwerte. Fachliche Begruendungen und Versuchsbeobachtungen bleiben
Teil der Schulaufgabe.

## Diagramme und Messreihen

- Im Ergebnis **Diagramm** oder EXE oeffnet die Grafik; **Tabelle** zeigt die Werte.
- Links/rechts bewegt den Cursor. Hoch/runter oder **Ansicht** waehlt weitere Grafiken.
- Ein Tippen in die Zeichenflaeche waehlt den naechsten Datenpunkt in x-Richtung.
- **Achsen** aendert Minima/Maxima in den angezeigten Einheiten. Minimum muss kleiner
  als Maximum sein; Abbruch erhaelt die vorige Ansicht und die Recheneingaben.
- Achsengrenzen bleiben fuer jede Grafik erhalten. **Auto** beziehungsweise `S`
  setzt die aktuelle Grafik auf automatische Skalierung zurueck.
- EXE wechselt zwischen Grafik und Tabelle. Zurueck fuehrt zum Ergebnis und dann
  wieder zu den unveraenderten Eingaben.

Allgemeine Messreihen umfassen 2-40 Punkte, Planck-Reihen weiterhin hoechstens zehn.
Messwerte werden manuell eingegeben; fotografierte Diagramme werden nicht erkannt.
Physikalische Vorlagen besitzen feste Achsengroessen und passende Einheiten.
Allgemeine Zahlenfits sind dimensionslose Auswertungen.
Fuer Ableitung und Integration muessen x-Werte streng aufsteigen. Flaechen sind
vorzeichenbehaftete Trapeznaeherungen; Steigungen sind Differenzenquotienten.

Fits sind ungewichtet. Statistische 1-sigma-Fehler ab drei Punkten beruhen auf
Residuen und Modellannahmen; sie sind keine vollstaendige experimentelle Unsicherheit.
Zwei Punkte liefern keinen Streuungsfehler. Nicht zulaessige Logarithmen und gleiche
x-Werte werden abgewiesen. Millikan-Vorlagen erlauben alle 32 Originalmessungen.

## Feste Simulationen

Enthalten sind die drei Modelle aus den Abituraufgaben Sachsen 2013 C1 (RL),
2014 C2 (Federwagen mit Reibung) und 2016 C1 (Fallkegel mit Luftwiderstand).
Parameter, Anfangswerte und Schrittweite sind einstellbar. Zusaetzliche Masken
passen die jeweils vorgesehene Groesse innerhalb eines eingegebenen Intervalls an.

RK4 und ein Kontrolllauf mit halber Schrittweite pruefen die numerische Aufloesung.
Maximal 50.000 Integrationsschritte insgesamt und hoechstens 400 Punkte je Kurve
begrenzen Rechenaufwand und Speicher. Wichtige Ereignisse werden innerhalb eines
Schritts lokalisiert; ein Wagen faehrt nach Stillstand nicht kuenstlich rueckwaerts.
Die 19 MoeBIUS-Projekte sind Referenzen, keine ausfuehrbaren Dateien dieser App.

Die elektrische Stromresonanz, die mechanische Auslenkungsresonanz und der
Scheitelwert einer Sinusschwingung sind verschiedene Groessen und werden getrennt benannt.

## Pruefstatus

Die native Basis wurde mit alpha.2 am Geraet bestaetigt: Dateien werden gefunden, Bilder laden schnell
und vollstaendig, die Navigation funktioniert gut. Es liegen keine gemessenen
Zeitwerte und keine vollstaendige Hardwarepruefung aller Funktionen vor.

Die vollstaendigen Menues und Rechnerfunktionen kamen mit alpha.3 hinzu.
**Eine vollstaendige Hardwarepruefung dieser Beta steht noch aus.**
Geeignete Stichproben:

1. **Physik-Rechner > Grundgroessen > T aus f**: `2 Hz` ergibt `0.5 s`.
2. **Quanten: Photonen > Photonenergie aus lambda**: `500 nm` ergibt etwa `2.47968 eV`.
3. Einheiten, Rueckkehr zu Eingaben sowie Fehlermeldung bei `f = 0` testen.
4. Planck-Messreihe `f / U_G`, zwei Punkte: `600 THz / 0.481400618 V` und
   `700 THz / 0.894967388 V` ergeben etwa `6.62607e-34 Js` und `2 eV`.
5. Beide Formelsuchen oeffnen und abschliessend ueber **Beenden** zum Launcher wechseln.

PC-Tests sind keine Hardwarefreigabe.

## Paket 1A testen

Unter **Physik-Rechner > Kinematik: beschleunigt** stehen am Ende der
Kategorienliste sieben Masken. Zeitangaben gelten seit Beginn der betrachteten
Phase; `x0` ist der Anfangsort auf einer gewaehlten positiven Achse. Negative
Geschwindigkeiten und Beschleunigungen sind erlaubt. Die Beschleunigung bleibt konstant.

| Maske | Eingabereihenfolge | Ausgabe |
| --- | --- | --- |
| Verlauf | x0, v0, a, t | x, Delta_x, v, Weglaenge |
| Zeit bis Ort | x0, v0, a, x | Alle zukuenftigen Zeiten mit Geschwindigkeit |
| Zeit bis Geschwindigkeit | v0, a, v | Zeit und Ortsaenderung |
| a aus Geschwindigkeiten | v0, v, t | a und Ortsaenderung |
| a aus Orten | x0, v0, t, x | a und Endgeschwindigkeit |
| v0 aus Orten | x0, a, t, x | Anfangs- und Endgeschwindigkeit |
| a aus Strecke | v0, v, Delta_x | a und Bewegungsdauer |

Laengen bieten m/km/cm/mm, Zeiten s/min/h/ms, Geschwindigkeiten m/s und km/h.
Ergebnisse erscheinen in SI-Einheiten. Einheitenwechsel aendert die Bedeutung,
nicht den eingegebenen Zahlenwert. Ortsaenderung ist vorzeichenbehaftet;
Weglaenge beruecksichtigt auch einen Richtungswechsel. Das allgemeine Modell
kann nach einem Stillstand umkehren; es ist noch kein Anhalteweg-Assistent.
Nahe Zeitloesungen werden mit erhoehter Anzeigepraezision unterschieden.
Reicht die Zahlenaufloesung fuer eine zuverlaessige Rechnung oder Anzeige
nicht aus, erscheint eine Meldung statt einer scheinbar eindeutigen Loesung.

Geraetestichproben (noch offen):

- Bob Nr. 14: Verlauf `0; 0; 1.8; 5` ergibt `x=22.5 m`, `v=9 m/s`, Weg `22.5 m`.
- Zeit bis Ort `0; 10; -2; 16`: `2 s` und `8 s`, Geschwindigkeiten `6` und `-6 m/s`.
- Gleiche Eingabe mit Zielort `26 m`: Fehlermeldung; danach auf `25 m` korrigieren.
- Gelbphase: a aus Strecke mit `54 km/h; 72 km/h; 25 m` ergibt `3.5 m/s^2`.
  Danach Zeit bis Ort `0; 15; 3.5; 37` ergibt `2 s` und `22 m/s`.
- Schnellzug I: a aus Strecke `120 km/h; 0; 4 km` ergibt `-0.138889 m/s^2`, `240 s`.
- Zum vorherigen Feld wechseln, DEL/Minus/E und Touch-Tasten pruefen,
  Ergebnisse scrollen, neu berechnen und ueber Beenden zum Launcher zurueckkehren.

Bob ist ein Originalfall aus `AB_Ueberholvorgang_GK`, PDF-Seite 4, Nr. 14a/b;
die zugehoerigen Inversen sind abgeleitete Rueckrechentests. Transrapid I,
Gelbphase und Schnellzug I stammen aus `UeA_geradlinige Bewegung`, PDF-Seite 2.
Bei Transrapid I werden Diagrammwerte weiterhin manuell abgelesen.
Fall-/Wurfmasken, Reaktionswege, Begegnungen und Diagramme sind seit beta.3 in
eigenen Kategorien zusaetzlich zu diesem unveraenderten 1A-Katalog enthalten.

Die Datenpfade werden aus dem vom Launcher übergebenen Programmpfad abgeleitet.
Ohne Startpfad wird das Hauptverzeichnis `\fls0\` verwendet.
Falls die Dateischnittstelle die ältere SDK-Pfadschreibweise benötigt,
wird diese nach einem pfadbezogenen Öffnungsfehler berücksichtigt. Eine fehlende
Dateigrößenabfrage verhindert das Lesen nicht; Paketkennung und begrenzte
Lesezugriffe bleiben geprüft. Bei Fehlern erscheinen Pfad, Schritt und Fehlercode.

## Lokal bauen

Die vollstaendige Regressionskette verwendet zusaetzlich lokale Unterrichtsdateien
und eingefrorene Vergleichspakete. Diese sind nicht Teil des Repositories.
Die nativen Quellen lassen sich mit dem unten genannten SDK separat ueber
`make hh3` bauen. Fuer die Installation sind die fertigen Release-Dateien vorgesehen.

Im Verzeichnis `native`:

```powershell
.\build.ps1 -Python <Pfad-zum-vorhandenen-Python>
```

Das Windows-Buildscript setzt einen Projektpfad auf E:, Python 3 mit Pillow,
die WSL-Distribution `kali-linux` mit C++-Host-Compiler und Docker sowie das
im Script festgelegte HH3-SDK-Image voraus. Das SDK-Image muss bereits lokal vorliegen.
Docker lädt keine Images herunter und hat beim Build keinen Netzwerkzugriff.
Temporäre Compilerdateien und Python-Ausgaben bleiben im Verzeichnis `native/build`.

Die Host-Tests vergleichen alle alten Datensaetze, 514 Kapitel, 39 Medien und die Rasterpixel
gegen die unveraenderte Python-Version. Zusaetzlich pruefen sie alle 546 Kapitel
ueber die neue Unterrichtsnavigation und
167 bisherige sowie sieben neue Rechnerformulare, Einheiten und alle vier Fit-Ablaeufe. Numerische Tests
vergleichen saemtliche festen Formeln mit der bisherigen Python-App und
unabhaengigen Quanten-Beispielwerten. AddressSanitizer, UndefinedBehaviorSanitizer
und LeakSanitizer pruefen auf Speicherfehler.
Die Kinematik hat unabhaengige Original-, Umkehr- und Grenzfalltests in
`tests/test_kinematics.cpp`; der Bericht steht in `dist/kinematics-tests.log`.
Native Binaerpruefungen kontrollieren Startcode,
Bildschirm-Sicherung und den zulässigen Programmspeicherbereich.
Ein separater SDK-Dateitest prüft kurze Lesezugriffe, fehlgeschlagene und falsch
gemeldete Positionierung, Ein-/Ausgabefehler und geschlossene Dateihandles.
Dieselben neuen Modul- und Oberflaechentests laufen ein zweites Mal mit den
datenbasierten Katalogen des Geraetebuilds. Ein eigener Vergleich prueft alle
254 Workflowdatensaetze bytegenau gegen den unabhaengigen Hostexport,
einschliesslich Namen, Formeln, Parametern, Einheiten, Vorgaben und Listenbegrenzungen.
Paketkennungen und Pruefsummen erkennen fehlende, gekuerzte oder veraenderte Kataloge.
Testberichte, Quellcode-Prüfsumme und Binärdaten stehen unter `dist`.

`sources/navigation.json` ist die zentrale Zuordnung fuer Oberthemen, Unterthemen,
Groessen und stabile Artikelkennungen. Der Build erzeugt daraus gepruefte,
byteindizierte Menues. Python- und C++-Tests kontrollieren Vollstaendigkeit,
Mehrfachverweise, beschaedigte Daten sowie Touch-/Tasten- und Ruecknavigation.
Der Hauptmenuepunkt **Formeln nach Groessen** entfaellt; Formeln bleiben ueber
**Formeln nach Themen** und **Formeln A-Z** erreichbar. Deren Verweise und Reihenfolge
sowie Theorie, Rechner und Abbildungen bleiben unveraendert. `wissen.txt` und
`bilder.dat` sind bytegleich zu Beta.5. Der Build prueft dies gegen die gesicherte
Beta.5 und kontrolliert weiterhin die eingefrorenen Referenzen aus Beta.3 und Beta.4.

## Quellen und Lizenzen

- Start-/Bildschirmkonzept, Schrift und ELF-Prüfung aus Mathe Vektor, Loewenkrieger.
- C++-Quellen unter GPL-3.0-or-later, siehe `LICENSE`.
- Hollyhock-3-SDK: Commit `1e483e395391d50d9bd2d21cee4beecc7589fb1c`,
  siehe `vendor/hollyhock-3/LICENSE`.
- DejaVu-Schrift: siehe `licenses/FONT-LICENSE.txt`.
