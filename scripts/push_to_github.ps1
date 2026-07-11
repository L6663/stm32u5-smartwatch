param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryUrl,

    [string]$Branch = "main",

    [string]$CommitMessage = "chore: import STM32U5 smartwatch alpha source"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

Write-Host "[1/6] Verifying repository..."
python .\scripts\verify_repo.py
if ($LASTEXITCODE -ne 0) {
    throw "Repository verification failed."
}

Write-Host "[2/6] Initializing Git repository..."
if (-not (Test-Path ".git")) {
    git init
}

git branch -M $Branch

Write-Host "[3/6] Staging files..."
git add .

Write-Host "[4/6] Creating commit..."
$HasHead = $true
git rev-parse --verify HEAD *> $null
if ($LASTEXITCODE -ne 0) {
    $HasHead = $false
}

$Pending = git status --porcelain
if ($Pending) {
    git commit -m $CommitMessage
} elseif (-not $HasHead) {
    throw "No files are available for the initial commit."
} else {
    Write-Host "No uncommitted changes; existing commit will be pushed."
}

Write-Host "[5/6] Configuring remote..."
$Origin = git remote get-url origin 2>$null
if ($LASTEXITCODE -eq 0) {
    if ($Origin -ne $RepositoryUrl) {
        git remote set-url origin $RepositoryUrl
    }
} else {
    git remote add origin $RepositoryUrl
}

Write-Host "[6/6] Pushing to GitHub..."
git push -u origin $Branch

Write-Host "Done."
