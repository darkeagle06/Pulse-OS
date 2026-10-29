Write-Host "=== Compilers ==="
$compilers = @("cl", "g++", "clang++")
foreach ($c in $compilers) {
    $found = Get-Command $c -ErrorAction SilentlyContinue
    if ($found) { Write-Host "$c : $($found.Source)" } else { Write-Host "$c : NOT FOUND" }
}

Write-Host "`n=== Build Tools ==="
$tools = @("cmake", "ninja", "make", "mingw32-make", "nmake")
foreach ($t in $tools) {
    $found = Get-Command $t -ErrorAction SilentlyContinue
    if ($found) { Write-Host "$t : $($found.Source)" } else { Write-Host "$t : NOT FOUND" }
}

Write-Host "`n=== Git ==="
$git = Get-Command git -ErrorAction SilentlyContinue
if ($git) { Write-Host "git : $($git.Source)"; & git --version } else { Write-Host "git : NOT FOUND" }

Write-Host "`n=== CMake Version ==="
$cm = Get-Command cmake -ErrorAction SilentlyContinue
if ($cm) { & cmake --version }

Write-Host "`n=== Qt Check ==="
$qm = Get-Command qmake -ErrorAction SilentlyContinue
if ($qm) { Write-Host "qmake : $($qm.Source)"; & qmake --version } else { Write-Host "qmake : NOT FOUND" }
$qtDirs = @("C:\Qt", "D:\Qt", "$env:USERPROFILE\Qt", "C:\Program Files\Qt", "C:\Program Files (x86)\Qt")
foreach ($d in $qtDirs) {
    if (Test-Path $d) {
        Write-Host "Found: $d"
        Get-ChildItem $d -Directory -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $_" }
    }
}

Write-Host "`n=== Visual Studio ==="
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $installs = & $vswhere -all -format json | ConvertFrom-Json
    foreach ($i in $installs) {
        Write-Host "  $($i.displayName) v$($i.installationVersion)"
        Write-Host "    Path: $($i.installationPath)"
        $msvcDir = "$($i.installationPath)\VC\Tools\MSVC"
        if (Test-Path $msvcDir) {
            $versions = Get-ChildItem $msvcDir -Directory | Select-Object -ExpandProperty Name
            Write-Host "    MSVC Toolsets: $($versions -join ', ')"
        }
    }
} else {
    Write-Host "vswhere not found"
}

Write-Host "`n=== System ==="
$os = Get-CimInstance Win32_OperatingSystem
Write-Host "OS: $($os.Caption) [$($os.Version)] $($os.OSArchitecture)"
$cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
Write-Host "CPU: $($cpu.Name)"
Write-Host "Cores: $($cpu.NumberOfCores) physical, $($cpu.NumberOfLogicalProcessors) logical"
Write-Host "RAM: $([math]::Round($os.TotalVisibleMemorySize / 1MB, 1)) GB"

Write-Host "`n=== Key Env Vars ==="
Write-Host "Qt6_DIR: $env:Qt6_DIR"
Write-Host "CMAKE_PREFIX_PATH: $env:CMAKE_PREFIX_PATH"
Write-Host "QTDIR: $env:QTDIR"

Write-Host "`n=== DONE ==="

