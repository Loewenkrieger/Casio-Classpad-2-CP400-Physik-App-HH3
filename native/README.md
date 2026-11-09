# Physik LK Native

Lokale Testversion `2.0.0-alpha.3` fuer Hollyhock 3.
Native C++20-App mit dem HH3-SDK und der Werkzeugbasis von Mathe Vektor.
Die App startet direkt als `physik.hh3`; PythonExtra wird nicht benoetigt.
Alle Quellen, Ausgaben und Sicherungen liegen auf E:. Keine GitHub-Veröffentlichung.

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

1. `dist/physik_native_2_0_0_alpha_3.zip` entpacken.
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

Beim Wechsel von alpha.2 sind die Datendateien unveraendert; nur `physik.hh3`
muss ersetzt werden. Das ZIP enthaelt dennoch alle drei passenden Dateien.
Der bestaetigte alpha.2-Stand bleibt unter `native-backups` gesichert.

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

Alpha.2 wurde am Geraet bestaetigt: Dateien werden gefunden, Bilder laden schnell
und vollstaendig, die Navigation funktioniert gut. Es liegen keine gemessenen
Zeitwerte und keine vollstaendige Hardwarepruefung aller Funktionen vor.

**Die neuen alpha.3-Funktionen muessen noch auf dem ClassPad geprueft werden.**
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

```powershell
.\build.ps1 -Python <Pfad-zum-vorhandenen-Python>
```

Der Build verwendet die vorhandene WSL-Distribution `kali-linux` unter `E:\WSL\Kali`,
den dort vorhandenen Host-Compiler und das lokal vorhandene, festgelegte SDK-Image.
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
