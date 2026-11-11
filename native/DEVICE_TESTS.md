# Geraetestand

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
