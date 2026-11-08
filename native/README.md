# Physik LK Native: lokaler Prototyp

Entwicklungsstand `2.0.0-alpha.2`, noch kein vollständiger Ersatz für die Python-App.
Native C++20-App mit dem HH3-SDK und der Werkzeugbasis von Mathe Vektor.
Alle Quellen, Ausgaben und Sicherungen liegen auf E:. Keine GitHub-Veröffentlichung.

## Verfügbar

- Hauptmenü, Touch-/Tastenbedienung, Scrollen und Beenden zum Launcher.
- Drei Themen: Materiewellen / Beugung, Doppelspalt sowie Emission / Spektren.
- Zugehörige Vektorskizzen sowie P4- und P8-Rasterbilder.
- Rückkehr aus der Bildansicht zur bisherigen Textposition.
- Native Leser für alle bestehenden Verzeichnisse, Kapitel und Medien.

Formelsuche und Rechner sind in diesem ersten Prototyp noch nicht umgesetzt.
Ihre Menüpunkte zeigen einen entsprechenden Hinweis, keine Beispielrechnung.

## Auf dem ClassPad testen

1. `dist/physik_native_2_0_0_alpha_2.zip` entpacken.
2. `physik.hh3`, `wissen.txt` und `bilder.dat` gemeinsam ins Hauptverzeichnis
   des Taschenrechners neben `run.bin` kopieren. PythonExtra ist nicht nötig.
3. Im HH3-Launcher **Physik LK Native** öffnen.
4. Unter Theorie **Materiewellen / Beugung** öffnen, etwas scrollen und EXE drücken.
5. Elektronenbeugungsröhre und Beugungsringe mit links/rechts durchblättern.
6. Mit Zurück/Clear zum Artikel zurückkehren; die Textposition soll erhalten bleiben.
7. Unter **Doppelspalt** die Punkt-/Streifenabbildung und unter **Emission / Spektren**
   das Linienspektrum prüfen. Bilder sollen vollständig und farblich korrekt sein.
8. Mehrfach zwischen Ansichten wechseln, danach über Beenden zum Launcher zurückkehren.

Bitte insbesondere Start, Schriftlesbarkeit, Bildfarben, vollständige Rasterbilder,
Rückkehr zum Text und Launcher prüfen. PC-Tests sind keine Hardwarefreigabe.

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

Die Host-Tests vergleichen alle Datensätze, 514 Kapitel, 39 Medien und die Rasterpixel
gegen die unveränderte Python-Version. Native Binärprüfungen kontrollieren Startcode,
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
