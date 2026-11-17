# sync-all.ps1 -- push this repo to Gitee (git) and GitHub (Git Data API) in one command
#
# Why: on this machine github.com is unreachable over HTTPS (DNS -> 20.6.7.17,
# TLS handshake times out), while api.github.com and ssh.github.com:443 work.
# So GitHub updates go through the REST API uploader in scripts/gh-upload.js.
#
# Usage:
#   .\scripts\sync-all.ps1                                  # use $env:GH_TOKEN, auto message
#   .\scripts\sync-all.ps1 -GitHubToken "ghp_xxx" -Message "fix: xxx"
#   .\scripts\sync-all.ps1 -SkipGitee                       # GitHub only
#   .\scripts\sync-all.ps1 -SkipGitHub                      # Gitee only

[CmdletBinding()]
param(
    [string]$GitHubToken = $env:GH_TOKEN,
    [string]$GitHubOwner = "fjasaffda",
    [string]$GitHubRepo  = "electronic-fence-lab",
    [string]$GiteeRemote = "gitee",
    [string]$Branch      = "main",
    [string]$Message     = ("sync " + (Get-Date -Format "yyyy-MM-dd HH:mm")),
    [switch]$SkipGitee,
    [switch]$SkipGitHub
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoDir   = Split-Path -Parent $scriptDir
Set-Location -LiteralPath $repoDir

function Resolve-Node {
    $cmd = Get-Command node -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $fallback = "$env:USERPROFILE\.dsh\dsh-runtimes\dsh-primary-runtime\dependencies\node\bin\node.exe"
    if (Test-Path $fallback) { return $fallback }
    throw "node not found; install Node.js or set PATH"
}

# ---- Gitee (plain git over HTTPS) ----
if (-not $SkipGitee) {
    Write-Host "[1/2] Gitee -> git push $GiteeRemote $Branch"
    & git push $GiteeRemote $Branch 2>&1 | ForEach-Object { Write-Host "  $_" }
    if ($LASTEXITCODE -ne 0) { Write-Host "  gitee push failed (check credentials: git credential fill)" }
    else { Write-Host "  gitee ok" }
}

# ---- GitHub (Git Data API) ----
if (-not $SkipGitHub) {
    if (-not $GitHubToken) {
        Write-Host "[2/2] GitHub skipped: no -GitHubToken and no `$env:GH_TOKEN"
    } else {
        Write-Host "[2/2] GitHub -> api.github.com Git Data API"
        $node = Resolve-Node
        $env:GH_TOKEN = $GitHubToken
        & $node "$scriptDir\gh-upload.js" $repoDir $GitHubOwner $GitHubRepo $Branch $Message 2>&1 |
            ForEach-Object { Write-Host "  $_" }
        if ($LASTEXITCODE -ne 0) { Write-Host "  github upload failed" } else { Write-Host "  github ok" }
    }
}

Write-Host ""
Write-Host "local HEAD: $((git log --oneline -1))"
Write-Host "gitee : https://gitee.com/mywuyu/$GitHubRepo"
Write-Host "github: https://github.com/$GitHubOwner/$GitHubRepo"