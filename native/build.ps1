param([string]$Python='python')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
if (-not $root.StartsWith('E:\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Diese Entwicklung muss auf E: liegen.' }
$project=Split-Path -Parent $root
$image='ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a'
foreach ($folder in @('build\tmp','build\home','dist')) { New-Item -ItemType Directory -Path (Join-Path $root $folder) -Force | Out-Null }
$oldTmp=$env:TMP
$oldTemp=$env:TEMP
$oldBytecode=$env:PYTHONDONTWRITEBYTECODE
$env:TMP=$env:TEMP=Join-Path $root 'build\tmp'
$env:PYTHONDONTWRITEBYTECODE='1'
Push-Location -LiteralPath $root
try {
  & $Python -B tools/generate_equations.py
  if ($LASTEXITCODE -ne 0) { throw 'Native Formelgenerierung fehlgeschlagen' }
  & $Python -B tools/reference.py
  if ($LASTEXITCODE -ne 0) { throw 'Referenzdaten fehlgeschlagen' }
  & $Python -B tools/generate_knowledge.py
  if ($LASTEXITCODE -ne 0) { throw 'Wissenserweiterung fehlgeschlagen' }
  $linuxRoot=(& wsl -d kali-linux -- wslpath -u $root).Trim()
  if ($LASTEXITCODE -ne 0) { throw 'WSL-Pfad nicht verfügbar' }
  $linuxProject=(& wsl -d kali-linux -- wslpath -u $project).Trim()
  if ($LASTEXITCODE -ne 0) { throw 'WSL-Projektpfad nicht verfügbar' }
  & wsl -d kali-linux --cd $linuxRoot -- env "TMPDIR=$linuxRoot/build/tmp" make build/export-catalog
  if ($LASTEXITCODE -ne 0) { throw 'Katalogexport-Build fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build/export-catalog
  if ($LASTEXITCODE -ne 0) { throw 'Katalogexport fehlgeschlagen' }
  & $Python -B tools/generate_catalog.py
  if ($LASTEXITCODE -ne 0) { throw 'Katalogdatendatei fehlgeschlagen' }
  & $Python -B tools/generate_navigation.py
  if ($LASTEXITCODE -ne 0) { throw 'Wissensnavigation fehlgeschlagen' }
  & $Python -B -m unittest discover -s tests -p test_navigation_data.py 2>&1 | Tee-Object -FilePath (Join-Path $root 'dist\navigation-data-tests.log')
  if ($LASTEXITCODE -ne 0) { throw 'Navigationsdaten-Tests fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env "TMPDIR=$linuxRoot/build/tmp" make host-test
  if ($LASTEXITCODE -ne 0) { throw 'Host-C++-Build fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build/host-tests 2>&1 | Tee-Object -FilePath (Join-Path $root 'dist\host-tests.log')
  if ($LASTEXITCODE -ne 0) { throw 'Native Host-Tests fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build/sdk-file-tests 2>&1 | Tee-Object -FilePath (Join-Path $root 'dist\sdk-file-tests.log')
  if ($LASTEXITCODE -ne 0) { throw 'Native Dateischnittstellen-Tests fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build/calculator-tests 2>&1 | Tee-Object -FilePath (Join-Path $root 'dist\calculator-tests.log')
  if ($LASTEXITCODE -ne 0) { throw 'Native Rechnertests fehlgeschlagen' }
  & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build/kinematics-tests 2>&1 | Tee-Object -FilePath (Join-Path $root 'dist\kinematics-tests.log')
  if ($LASTEXITCODE -ne 0) { throw 'Native Kinematiktests fehlgeschlagen' }
  foreach ($suite in @('mechanics','fields','optics','analysis','simulation','extensions','navigation')) {
    & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "./build/$suite-tests" 2>&1 | Tee-Object -FilePath (Join-Path $root "dist\$suite-tests.log")
    if ($LASTEXITCODE -ne 0) { throw "Native Tests fehlgeschlagen: $suite" }
  }
  & wsl -d kali-linux --cd $linuxRoot -- env "TMPDIR=$linuxRoot/build/tmp" make compact-test
  if ($LASTEXITCODE -ne 0) { throw 'Build der datenbasierten Katalogpruefung fehlgeschlagen' }
  foreach ($suite in @('host','mechanics','fields','optics','analysis','simulation','extensions','catalog','navigation')) {
    & wsl -d kali-linux --cd $linuxRoot -- env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "./build/compact-$suite-tests" 2>&1 | Tee-Object -FilePath (Join-Path $root "dist\compact-$suite-tests.log")
    if ($LASTEXITCODE -ne 0) { throw "Datenbasierte Katalogpruefung fehlgeschlagen: $suite" }
  }
  & wsl -d kali-linux -u root -- docker run --pull never --rm --read-only --network none --cap-drop ALL --security-opt no-new-privileges --mount "type=bind,source=$linuxProject,target=/project" --env TMPDIR=/project/native/build/tmp --env HOME=/project/native/build/home --workdir /project/native $image make hh3
  if ($LASTEXITCODE -ne 0) { throw 'HH3-Cross-Build fehlgeschlagen' }
  & $Python -B tools/verify.py
  if ($LASTEXITCODE -ne 0) { throw 'Paket- oder Binaerpruefung fehlgeschlagen' }
} finally {
  Pop-Location
  $env:TMP=$oldTmp
  $env:TEMP=$oldTemp
  $env:PYTHONDONTWRITEBYTECODE=$oldBytecode
}
