param(
    [ValidateSet('Release','Debug')][string]$Configuration = 'Release',
    [ValidateRange(1,32)][int]$Parallel = 2
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDir = Join-Path $projectRoot 'build'
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    $vswhere = Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $vsRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsRoot) {
            $candidate = Join-Path $vsRoot 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
            if (Test-Path -LiteralPath $candidate) { $cmake = Get-Item -LiteralPath $candidate }
        }
    }
}
if (-not $cmake) { throw 'CMake is missing. Install CMake 3.24+ and add it to PATH, or install the Visual Studio C++ CMake tools.' }
$cmakeExe = if ($cmake -is [System.Management.Automation.CommandInfo]) { $cmake.Source } else { $cmake.FullName }
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { throw 'Git is missing. Install Git for Windows and reopen PowerShell.' }
& $cmakeExe -S $projectRoot -B $buildDir -G 'Visual Studio 17 2022' -A x64 -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw 'Configuration failed. Check Visual Studio 2022 C++ tools and network access to GitHub.' }
& $cmakeExe --build $buildDir --config $Configuration --parallel $Parallel
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
$ctestExe = Join-Path (Split-Path $cmakeExe -Parent) 'ctest.exe'
& $ctestExe --test-dir $buildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'DSP tests failed; package was not produced.' }
& (Join-Path $PSScriptRoot 'package-windows.ps1') -Configuration $Configuration
