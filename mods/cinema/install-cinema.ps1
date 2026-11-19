<#
.SYNOPSIS
  Installe Cinema et KaeGae Core (et seulement eux) dans l'émulateur, pour Zelda Ocarina of Time (Switch 2).

.DESCRIPTION
  Télécharge le dernier cinema-pack.zip des Releases GitHub du projet (ou prend celui posé à côté du
  script, ou celui donné avec -Pack), puis copie KaeGae Core et Cinema dans le dossier de mods de
  l'émulateur (Fritchii, Eden ou Lichii). Une ancienne version de l'un ou de l'autre est remplacée.
  Si un autre mod occupe déjà l'emplacement de module de Cinema, Cinema en prend un libre.

  Fermer l'émulateur avant de lancer le script.

.PARAMETER LoadDir
  Le dossier "load" de l'émulateur, s'il n'est pas trouvé tout seul (exemple : "$env:APPDATA\fritchii\load").

.PARAMETER Pack
  Un cinema-pack.zip déjà téléchargé, au lieu de le télécharger.

.PARAMETER DesactiverLesAutres
  Met de côté les autres mods du jeu (dans "load-desactives", à côté de "load"), pour ne garder que Core et Cinema.

.PARAMETER ReactiverLesAutres
  Remet les mods mis de côté par -DesactiverLesAutres.

.PARAMETER Desinstaller
  Retire Cinema, et KaeGae Core si plus aucun autre mod n'est installé pour le jeu.

.EXAMPLE
  .\install-cinema.ps1
.EXAMPLE
  .\install-cinema.ps1 -DesactiverLesAutres
#>
param(
    [string]$LoadDir,
    [string]$Pack,
    [switch]$DesactiverLesAutres,
    [switch]$ReactiverLesAutres,
    [switch]$Desinstaller
)

$ErrorActionPreference = 'Stop'
$Title = '0400486024DEC000'
$Repo = 'seitanmeitoo/oot-remake-modding'
$CoreId = 'kaegae.core'
$CinemaId = 'seitanmeitoo.cinema'
$Utf8 = New-Object System.Text.UTF8Encoding $false

function Stop-Install([string]$message) {
    Write-Host $message -ForegroundColor Red
    exit 1
}

# L'identifiant d'un dossier de mod (id = "..." de son mod.toml), ou $null.
function Get-ModId([string]$folder) {
    $toml = Join-Path $folder 'mod.toml'
    if (-not (Test-Path -LiteralPath $toml)) { return $null }
    $m = [regex]::Match([IO.File]::ReadAllText($toml), '(?m)^\s*id\s*=\s*"([^"]+)"')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

function Get-ModFolders([string]$dir) {
    if (-not (Test-Path -LiteralPath $dir)) { return @() }
    return @(Get-ChildItem -LiteralPath $dir -Directory)
}

# ---- le dossier de l'émulateur ----

if (-not $LoadDir) {
    $found = @('fritchii', 'eden', 'lichii') |
        ForEach-Object { Join-Path $env:APPDATA "$_\load" } |
        Where-Object { Test-Path -LiteralPath $_ }
    if (-not $found) {
        Stop-Install "Aucun émulateur trouvé dans $env:APPDATA (fritchii, eden, lichii). Donne son dossier avec -LoadDir."
    }
    $LoadDir = @($found)[0]
}
$GameDir = Join-Path $LoadDir $Title
$AsideDir = Join-Path (Split-Path $LoadDir -Parent) "load-desactives\$Title"
Write-Host "Dossier des mods : $GameDir"

$running = Get-Process -Name 'fritchii', 'eden', 'lichii', 'yuzu' -ErrorAction SilentlyContinue
if ($running) {
    Stop-Install "L'émulateur est ouvert ($($running[0].ProcessName)). Ferme-le puis relance le script."
}
New-Item -ItemType Directory -Force -Path $GameDir | Out-Null

# ---- remettre les mods mis de côté ----

if ($ReactiverLesAutres) {
    foreach ($f in Get-ModFolders $AsideDir) {
        $dest = Join-Path $GameDir $f.Name
        if (Test-Path -LiteralPath $dest) {
            Write-Host "  $($f.Name) : déjà présent, laissé de côté." -ForegroundColor Yellow
            continue
        }
        Move-Item -LiteralPath $f.FullName -Destination $dest
        Write-Host "  réactivé : $($f.Name)"
    }
    Write-Host 'Terminé.' -ForegroundColor Green
    exit 0
}

# ---- désinstaller ----

if ($Desinstaller) {
    foreach ($f in Get-ModFolders $GameDir) {
        if ((Get-ModId $f.FullName) -eq $CinemaId) {
            Remove-Item -LiteralPath $f.FullName -Recurse -Force
            Write-Host "  retiré : $($f.Name)"
        }
    }
    $others = @(Get-ModFolders $GameDir | Where-Object { (Get-ModId $_.FullName) -ne $CoreId })
    if ($others.Count -eq 0) {
        foreach ($f in Get-ModFolders $GameDir) {
            Remove-Item -LiteralPath $f.FullName -Recurse -Force
            Write-Host "  retiré : $($f.Name) (plus aucun mod ne s'en sert)"
        }
    } else {
        Write-Host "  KaeGae Core gardé : d'autres mods sont installés ($($others.Name -join ', '))."
    }
    Write-Host 'Terminé.' -ForegroundColor Green
    exit 0
}

# ---- le paquet ----

$Work = Join-Path ([IO.Path]::GetTempPath()) ("cinema-install-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $Work | Out-Null
try {
    if (-not $Pack -and $PSScriptRoot) {
        $local = Join-Path $PSScriptRoot 'cinema-pack.zip'
        if (Test-Path -LiteralPath $local) { $Pack = $local }
    }
    if (-not $Pack) {
        [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
        Write-Host 'Recherche de la dernière version...'
        $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest" -Headers @{ 'User-Agent' = 'cinema-installer' }
        $asset = $release.assets | Where-Object { $_.name -eq 'cinema-pack.zip' } | Select-Object -First 1
        if (-not $asset) { Stop-Install "La version $($release.tag_name) n'a pas de cinema-pack.zip." }
        $Pack = Join-Path $Work 'cinema-pack.zip'
        Write-Host "Téléchargement de $($release.tag_name)..."
        Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $Pack -UseBasicParsing -Headers @{ 'User-Agent' = 'cinema-installer' }
    }
    $Unpacked = Join-Path $Work 'pack'
    Expand-Archive -LiteralPath $Pack -DestinationPath $Unpacked
    $PackGame = Join-Path $Unpacked $Title
    $newMods = @(Get-ModFolders $PackGame | Where-Object { Get-ModId $_.FullName })
    if ($newMods.Count -eq 0) { Stop-Install "Le paquet ne contient aucun mod dans $Title." }
    $newIds = @($newMods | ForEach-Object { Get-ModId $_.FullName })

    # ---- mettre les autres de côté ----

    if ($DesactiverLesAutres) {
        foreach ($f in Get-ModFolders $GameDir) {
            if ($newIds -contains (Get-ModId $f.FullName)) { continue }
            New-Item -ItemType Directory -Force -Path $AsideDir | Out-Null
            $dest = Join-Path $AsideDir $f.Name
            if (Test-Path -LiteralPath $dest) { Remove-Item -LiteralPath $dest -Recurse -Force }
            Move-Item -LiteralPath $f.FullName -Destination $dest
            Write-Host "  mis de côté : $($f.Name)"
        }
    }

    # ---- les emplacements de module (subsdk1 est celui de Core) ----

    $taken = @{}
    foreach ($f in Get-ModFolders $GameDir) {
        if ($newIds -contains (Get-ModId $f.FullName)) { continue }   # remplacé plus bas
        $exefs = Join-Path $f.FullName 'exefs'
        if (Test-Path -LiteralPath $exefs) {
            Get-ChildItem -LiteralPath $exefs -File -Filter 'subsdk*' | ForEach-Object { $taken[$_.Name] = $f.Name }
        }
    }

    foreach ($mod in $newMods) {
        $id = Get-ModId $mod.FullName
        $exefs = Join-Path $mod.FullName 'exefs'
        if ($id -ne $CoreId -and (Test-Path -LiteralPath $exefs)) {
            foreach ($slot in @(Get-ChildItem -LiteralPath $exefs -File -Filter 'subsdk*')) {
                if (-not $taken.ContainsKey($slot.Name)) { $taken[$slot.Name] = $mod.Name; continue }
                $free = 2..8 | ForEach-Object { "subsdk$_" } | Where-Object { -not $taken.ContainsKey($_) } | Select-Object -First 1
                if (-not $free) { Stop-Install "Plus d'emplacement libre (subsdk2 à subsdk8) pour $($mod.Name). Utilise -DesactiverLesAutres." }
                Rename-Item -LiteralPath $slot.FullName -NewName $free
                $toml = Join-Path $mod.FullName 'mod.toml'
                $text = [IO.File]::ReadAllText($toml) -replace "(?m)^(\s*)$($slot.Name)(\s*=)", "`${1}$free`${2}"
                [IO.File]::WriteAllText($toml, $text, $Utf8)
                $taken[$free] = $mod.Name
                Write-Host "  $($mod.Name) : $($slot.Name) est pris par $($taken[$slot.Name]), son module va dans $free"
            }
        }

        # Remplacer l'ancienne version (reconnue à son identifiant, quel que soit le nom du dossier).
        foreach ($f in Get-ModFolders $GameDir) {
            if ((Get-ModId $f.FullName) -eq $id) { Remove-Item -LiteralPath $f.FullName -Recurse -Force }
        }
        Copy-Item -LiteralPath $mod.FullName -Destination (Join-Path $GameDir $mod.Name) -Recurse
        $version = [regex]::Match([IO.File]::ReadAllText((Join-Path $mod.FullName 'mod.toml')), '(?m)^\s*version\s*=\s*"([^"]+)"').Groups[1].Value
        Write-Host "  installé : $($mod.Name) $version"
    }
}
finally {
    Remove-Item -LiteralPath $Work -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host ''
Write-Host 'Terminé. Lance le jeu : le menu des mods s''ouvre avec - et + ensemble, le panneau Cinema avec L + R.' -ForegroundColor Green
Write-Host 'Pour le panneau souris, active la souris dans Fritchii (réglages des contrôles, onglet Avancé).'
