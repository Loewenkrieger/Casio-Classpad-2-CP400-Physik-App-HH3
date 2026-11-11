# Physik LK Native

Native Beta-Version `2.0.0-beta.1` fuer Hollyhock 3.
Native C++20-App mit dem HH3-SDK und der Werkzeugbasis von Mathe Vektor.
Die App startet direkt als `physik.hh3`; PythonExtra wird nicht benoetigt.

[Beta herunterladen](https://github.com/Loewenkrieger/Casio-Classpad-2-CP400-Physik-App-HH3/releases/tag/v2.0.0-beta.1).
Diese Vorabversion ist noch kein stabiler Release. Die bisherige Python-Version
bleibt separat als v1.0.0 verfuegbar.

## Verfügbar

- Hauptmenü, Touch-/Tastenbedienung, Scrollen und Beenden zum Launcher.
- Theorie und Formelsuche nach Groessen und Themen: alle 840 bestehenden
  Menueverweise auf 514 Wissenskapitel, ohne inhaltliche Aenderungen.
- Alle 39 oeffentlichen Abbildungen: Vektorskizzen sowie P4-/P8-Rasterbilder.
- Rückkehr aus der Bildansicht zur bisherigen Textposition.
- Native Leser für alle bestehenden Verzeichnisse, Kapitel und Medien.

- 17 Rechnerkategorien mit 163 festen Formeln und vier Planck-Messreihen:
  je zwei Punkte oder 3-10 Punkte fuer `f / U_G` und `lambda / E_kin`.
- Touch-Zahlentastatur, wissenschaftliche Schreibweise und Einheitenwahl.

`wissen.txt` und `bilder.dat` sind bytegleich zum bisherigen oeffentlichen
Python-Build. Die Formeln werden beim Bauen in festen C++-Code uebersetzt;
auf dem ClassPad gibt es weder Python-Ausfuehrung noch `eval()`.

## Auf dem ClassPad testen

1. `physik_hh3_native_v2_0_0_beta_1.zip` aus dem Beta-Release herunterladen und entpacken.
2. `physik.hh3`, `wissen.txt` und `bilder.dat` gemeinsam ins Hauptverzeichnis
   des Taschenrechners neben `run.bin` kopieren. PythonExtra ist nicht nötig.
3. Im HH3-Launcher **Physik LK Native** öffnen.
4. Unter Theorie **Quanten: Materiewellen / Beugung > De Broglie und Modellgrenzen**
   oeffnen, etwas scrollen und EXE druecken.
5. Elektronenbeugungsröhre und Beugungsringe mit links/rechts durchblättern.
6. Mit Zurück/Clear zum Artikel zurückkehren; die Textposition soll erhalten bleiben.
7. Unter **Doppelspalt mit Quantenobjekten** die Punkt-/Streifenabbildung und unter
   **Atom: Spektren / Lichtquellen > Emission, Absorption und Serien** das
   Linienspektrum pruefen. Bilder sollen vollstaendig und farblich korrekt sein.
8. Mehrfach zwischen Ansichten wechseln, danach über Beenden zum Launcher zurückkehren.

Das ZIP enthaelt genau `physik.hh3`, `wissen.txt` und `bilder.dat`.
Alle drei Dateien gehoeren gemeinsam in denselben Ordner. Fuer die erste
Beta-Installation das vollstaendige Paket verwenden. `cinput.py`, `gint.py`
und PythonExtra sind nicht erforderlich.

## Rechner bedienen

Werte einzeln eingeben und **Weiter** oder EXE druecken. Links/hoch fuehrt zum
vorherigen Feld. **Rechnen** berechnet das Ergebnis. **Einheit** wechselt z.B.
zwischen m/nm/pm oder J/eV/keV. Der Zahlenwert bleibt dabei gleich; seine
Interpretation aendert sich. `E` erlaubt Eingaben wie `6.626E-34`, **DEL** loescht.
Nach einem Ergebnis ersetzt die erste neue Ziffer den bisherigen Feldinhalt.

Bei Messreihen sind THz, nm und eV vorbelegt. Der ungewichtete Fit zeigt `h`,
Austrittsarbeit, Grenzfrequenz und RMS, aber keine Messunsicherheit.
Die hinterlegten Konstanten bleiben unveraendert.

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

Die Datenpfade werden aus dem vom Launcher übergebenen Programmpfad abgeleitet.
Ohne Startpfad wird das Hauptverzeichnis `\fls0\` verwendet.
Falls die Dateischnittstelle die ältere SDK-Pfadschreibweise benötigt,
wird diese nach einem pfadbezogenen Öffnungsfehler berücksichtigt. Eine fehlende
Dateigrößenabfrage verhindert das Lesen nicht; Paketkennung und begrenzte
Lesezugriffe bleiben geprüft. Bei Fehlern erscheinen Pfad, Schritt und Fehlercode.

## Lokal bauen

Im Verzeichnis `native`:

```powershell
.\build.ps1 -Python <Pfad-zum-vorhandenen-Python>
```

Das Windows-Buildscript setzt einen Projektpfad auf E:, Python 3 mit Pillow,
die WSL-Distribution `kali-linux` mit C++-Host-Compiler und Docker sowie das
im Script festgelegte HH3-SDK-Image voraus. Das SDK-Image muss bereits lokal vorliegen.
Docker lädt keine Images herunter und hat beim Build keinen Netzwerkzugriff.
Temporäre Compilerdateien und Python-Ausgaben bleiben im Verzeichnis `native/build`.

Die Host-Tests vergleichen alle Datensaetze, 514 Kapitel, 39 Medien und die Rasterpixel
gegen die unveraenderte Python-Version. Sie pruefen alle 840 Artikelverweise,
167 Rechnerformulare, Einheiten und alle vier Fit-Ablaeufe. Numerische Tests
vergleichen saemtliche festen Formeln mit der bisherigen Python-App und
unabhaengigen Quanten-Beispielwerten. AddressSanitizer, UndefinedBehaviorSanitizer
und LeakSanitizer pruefen auf Speicherfehler.
Native Binaerpruefungen kontrollieren Startcode,
Bildschirm-Sicherung und den zulässigen Programmspeicherbereich.
Ein separater SDK-Dateitest prüft kurze Lesezugriffe, fehlgeschlagene und falsch
gemeldete Positionierung, Ein-/Ausgabefehler und geschlossene Dateihandles.
Testberichte, Quellcode-Prüfsumme und Binärdaten stehen unter `dist`.

## Quellen und Lizenzen

- Start-/Bildschirmkonzept, Schrift und ELF-Prüfung aus Mathe Vektor, Loewenkrieger.
- C++-Quellen unter GPL-3.0-or-later, siehe `LICENSE`.
- Hollyhock-3-SDK: Commit `1e483e395391d50d9bd2d21cee4beecc7589fb1c`,
  siehe `vendor/hollyhock-3/LICENSE`.
- DejaVu-Schrift: siehe `licenses/FONT-LICENSE.txt`.
