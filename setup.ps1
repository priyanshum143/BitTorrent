param(
  [switch]$RunDownload = $false,
  [string]$Torrent = "",
  [string]$Out = "out.bin"
)

$ErrorActionPreference = "Stop"

function Has-Wsl {
  try {
    wsl --status | Out-Null
    return $true
  } catch {
    return $false
  }
}

if (-not (Has-Wsl)) {
  Write-Host "[setup] WSL is not installed or not available."
  Write-Host "        Recommended: install WSL + Ubuntu, then re-run this script."
  Write-Host ""
  Write-Host "Steps (admin PowerShell):"
  Write-Host "  wsl --install"
  Write-Host "  (reboot if prompted)"
  Write-Host "  Launch Ubuntu once and create a user"
  exit 1
}

# Ensure Ubuntu exists (best-effort; if user has another distro, they can adapt)
Write-Host "[setup] Using WSL. Building inside Linux..."

# Install deps + build inside WSL
$repoPath = (Resolve-Path ".").Path
# Convert Windows path -> WSL path
$wslRepoPath = wsl wslpath -a "$repoPath"
$buildCmd = @"
set -euo pipefail
cd "$wslRepoPath"
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libcurl4-openssl-dev libssl-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
echo "[setup] Built: build/bt_main"
"@

wsl bash -lc $buildCmd

if ($RunDownload) {
  if ([string]::IsNullOrWhiteSpace($Torrent)) {
    Write-Host "[setup] --Torrent is required with -RunDownload"
    exit 1
  }

  # Convert torrent path to WSL path if it's a Windows path
  $torrentPath = (Resolve-Path $Torrent).Path
  $wslTorrentPath = wsl wslpath -a "$torrentPath"
  $wslOutPath = wsl wslpath -a (Join-Path $repoPath $Out)

  $runCmd = @"
set -euo pipefail
cd "$wslRepoPath"
./build/bt_main download -o "$wslOutPath" "$wslTorrentPath"
echo "[setup] Download finished: $wslOutPath"
"@
  wsl bash -lc $runCmd
}

Write-Host "[setup] Done."
