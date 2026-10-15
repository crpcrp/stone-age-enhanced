# Build with the CMake presets (CMakePresets.json), finding the CMake that
# Visual Studio installs when none is on PATH. Outputs land in build/.
#   .\build.ps1                 player with the default toolchain (Visual Studio)
#   .\build.ps1 -Compiler g++   use a GCC/Clang compiler through Ninja instead
#   .\build.ps1 -CoreOnly       portable core and asset packer only, no SDL player
#   .\build.ps1 -Test           also build and run the tests (needs research/ and original_data/)
param([string]$Compiler, [switch]$CoreOnly, [switch]$Test)
function Find-Tool([string]$Name) {
    $found = Get-Command $Name -ErrorAction SilentlyContinue
    if ($found) { return $found.Source }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere) {
        foreach ($root in (& $vswhere -all -products * -property installationPath)) {
            $candidate = Join-Path $root "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\$Name.exe"
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }
    throw "$Name was not found. Install CMake 3.25 or newer, or the Visual Studio C++ CMake tools."
}
Push-Location -LiteralPath $PSScriptRoot
try {
    $cmake = Find-Tool 'cmake'
    $preset = if ($Test) { 'test' } elseif ($CoreOnly) { 'core' } elseif ($Compiler) { 'ninja' } else { 'default' }
    $configure = @('--preset', $preset)
    if ($Compiler) { $configure += "-DCMAKE_CXX_COMPILER=$Compiler" }
    & $cmake @configure
    if ($LASTEXITCODE) { throw 'CMake configuration failed' }
    & $cmake --build --preset $preset --parallel
    if ($LASTEXITCODE) { throw 'Build failed' }
    if ($Test) {
        & (Find-Tool 'ctest') --preset test --parallel ([Environment]::ProcessorCount)
        if ($LASTEXITCODE) { throw 'Tests failed' }
    }
    Write-Host 'Reconstruction build completed.'
} finally { Pop-Location }
