param([switch]$Simulator, [switch]$InputTrace)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$env:IDF_TOOLS_PATH = 'C:\Users\User\.espressif'
$idfExport = 'C:\esp\v5.5.3\esp-idf\export.ps1'
if (-not (Test-Path -LiteralPath $idfExport)) { throw "ESP-IDF 5.5.3 is required: $idfExport" }
$env:Path = 'C:\Users\User\.espressif\python_env\idf5.5_py3.13_env\Scripts;' + $env:Path
. $idfExport
idf.py --version
if ($LASTEXITCODE -ne 0) { throw 'ESP-IDF activation failed' }
$buildDir = if ($Simulator) { 'build-sim' } else { 'build' }
$testFlag = if ($Simulator) { '-DFC_SIM_TEST=ON' } else { '-DFC_SIM_TEST=OFF' }
$traceFlag = if ($InputTrace) { '-DFC_INPUT_TRACE=ON' } else { '-DFC_INPUT_TRACE=OFF' }
idf.py --ccache -B $buildDir $testFlag $traceFlag -DFC_WIFI_MEMORY_TEST=OFF build
if ($LASTEXITCODE -ne 0) { throw 'Firmware build failed' }
if (-not $Simulator) {
    & '.\.venv\Scripts\python.exe' tools/package_firmware.py
    if ($LASTEXITCODE -ne 0) { throw 'Firmware packaging failed' }
}
