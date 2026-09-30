param([string]$GameDirectory = (Split-Path -Parent $PSScriptRoot))
$ErrorActionPreference = 'Stop'
function Get-SaveHash([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-', '') }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}
$gameRoot = [IO.Path]::GetFullPath($GameDirectory)
if (-not (Test-Path -LiteralPath (Join-Path $gameRoot 'vehiclethief-client.json'))) {
    throw 'This helper requires the isolated VehicleThief TS Client test folder.'
}
$gameExecutable = Join-Path $gameRoot 'game.exe'
if (@(Get-Process -Name game -ErrorAction SilentlyContinue | Where-Object { $_.Path -and [string]::Equals($_.Path, $gameExecutable, [StringComparison]::OrdinalIgnoreCase) }).Count -gt 0) {
    throw 'Exit the game before registering its multiplayer saves.'
}
$saveRoot = Join-Path $gameRoot 'Saved Games'
New-Item -ItemType Directory -Path $saveRoot -Force | Out-Null
$sources = @(Get-ChildItem -LiteralPath $saveRoot -File | Where-Object { $_.Name -cmatch '^SAVEGAME_[0-9]{3}\.NET$' } | Sort-Object Name)
if ($sources.Count -eq 0) { Write-Output 'No Vinifera multiplayer saves to register.'; exit 0 }
if (-not (Test-Path -LiteralPath (Join-Path $saveRoot 'spawnSG.ini'))) { throw 'Saved multiplayer lobby settings spawnSG.ini are missing.' }
$changes = @()
$expectedAliases = @($sources | ForEach-Object { 'SVGM_{0:D3}.NET' -f [int]$_.BaseName.Substring(9) })
foreach ($existing in @(Get-ChildItem -LiteralPath $saveRoot -File | Where-Object { $_.Name -cmatch '^SVGM_[0-9]{3}\.NET$' -and $_.Name -notin $expectedAliases })) {
    # Keep older client slots outside the current session's contiguous list.
    # This is a file move between verified paths under the isolated game root.
    $backupRoot = Join-Path $gameRoot ('MP-save-backups\' + (Get-Date -Format 'yyyyMMdd-HHmmss-fffffff'))
    if (-not $existing.FullName.StartsWith($saveRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected save path.' }
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    Move-Item -LiteralPath $existing.FullName -Destination (Join-Path $backupRoot $existing.Name)
}
foreach ($source in $sources) {
    $slot = [int]$source.BaseName.Substring(9)
    $target = Join-Path $saveRoot ('SVGM_{0:D3}.NET' -f $slot)
    $sourceHash = Get-SaveHash $source.FullName
    if (Test-Path -LiteralPath $target) {
        if ((Get-SaveHash $target) -eq $sourceHash) { continue }
        # Preserve an existing client save before replacing this alias. Original
        # Vinifera saves are never renamed, moved, or deleted.
        $backupRoot = Join-Path $gameRoot ('MP-save-backups\' + (Get-Date -Format 'yyyyMMdd-HHmmss-fffffff'))
        New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
        Copy-Item -LiteralPath $target -Destination (Join-Path $backupRoot ([IO.Path]::GetFileName($target)))
    }
    Copy-Item -LiteralPath $source.FullName -Destination $target -Force
    if ((Get-SaveHash $target) -ne $sourceHash) { throw 'Save copy hash check failed.' }
    [IO.File]::SetLastWriteTimeUtc($target, $source.LastWriteTimeUtc)
    $changes += [pscustomobject]@{ source = $source.Name; client_alias = [IO.Path]::GetFileName($target); sha256 = $sourceHash.ToLowerInvariant() }
}
$record = [pscustomobject]@{ original_files_preserved = $true; copies = $changes; source_count = $sources.Count }
$record | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $saveRoot 'mp-save-registration.json') -Encoding UTF8
Write-Output ('Registered {0} multiplayer saves for the client; originals preserved. Restart TS Client before loading.' -f $sources.Count)
