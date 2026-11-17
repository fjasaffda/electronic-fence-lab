# push-all.ps1 -- create + push this repo to Gitee and GitHub in one shot
# Usage (PowerShell 5.1 or 7):
#   .\scripts\push-all.ps1 -GiteeToken "<GITEE_TOKEN>" -GitHubToken "<GITHUB_PAT>" `
#                          [-GiteeOwner mywuyu] [-GitHubOwner <your-login>] `
#                          [-RepoName electronic-fence-lab] [-Private]
# Token scope:
#   Gitee  : personal access token with "projects" read+write
#            https://gitee.com/profile/personal_access_tokens
#   GitHub : classic PAT with "repo", or fine-grained with
#            Contents:write + Administration:write
#            https://github.com/settings/tokens
# The script removes the token from the remote URL right after pushing,
# so no secret is left inside .git/config.

[CmdletBinding()]
param(
    [string]$GiteeToken  = $env:GITEE_TOKEN,
    [string]$GitHubToken = $env:GITHUB_TOKEN,
    [string]$GiteeOwner  = "mywuyu",
    [string]$GitHubOwner = "",
    [string]$RepoName    = "electronic-fence-lab",
    [string]$Description = "Electronic fence lab: STM32L431 + Huawei LiteOS + ESP8285 + Huawei OceanConnect IoT",
    [switch]$Private
)

$ErrorActionPreference = "Stop"

# repo root = parent of the folder holding this script
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoDir   = Split-Path -Parent $scriptDir
Set-Location -LiteralPath $repoDir
Write-Host "repo: $repoDir"

function Set-RemoteUrl([string]$name, [string]$url) {
    $remotes = git remote
    if ($remotes -contains $name) { git remote set-url $name $url } else { git remote add $name $url }
}

# ---------------- Gitee ----------------
if ($GiteeToken) {
    $payload = @{
        name        = $RepoName
        description = $Description
        private     = $(if ($Private) { "true" } else { "false" })
        auto_init   = "false"
        has_issues  = "true"
    }
    try {
        $r = Invoke-RestMethod -Method Post -TimeoutSec 30 `
            -Uri "https://gitee.com/api/v5/user/repos?access_token=$GiteeToken" `
            -Body $payload -ContentType "application/x-www-form-urlencoded"
        Write-Host "Gitee repo created: $($r.full_name)"
    } catch {
        $msg = $_.ErrorDetails.Message
        if ($msg -match "already exists|has already been taken") { Write-Host "Gitee repo already exists, pushing" }
        else { Write-Host "Gitee create failed: $msg" }
    }
    Set-RemoteUrl "gitee" "https://$GiteeOwner`:$GiteeToken@gitee.com/$GiteeOwner/$RepoName.git"
    & git push -u gitee main 2>&1 | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "gitee push failed" }
    git remote set-url gitee "https://gitee.com/$GiteeOwner/$RepoName.git"
    Write-Host "Gitee done: https://gitee.com/$GiteeOwner/$RepoName"
}

# ---------------- GitHub ----------------
if ($GitHubToken) {
    $h = @{ Authorization = "token $GitHubToken"; "User-Agent" = "dsh-push-script"; Accept = "application/vnd.github+json" }
    if (-not $GitHubOwner) {
        $GitHubOwner = (Invoke-RestMethod "https://api.github.com/user" -Headers $h -TimeoutSec 30).login
    }
    $body = @{ name = $RepoName; description = $Description; private = [bool]$Private; auto_init = $false } | ConvertTo-Json
    try {
        $r = Invoke-RestMethod -Method Post -Uri "https://api.github.com/user/repos" -Headers $h -Body $body -ContentType "application/json" -TimeoutSec 30
        Write-Host "GitHub repo created: $($r.full_name)"
    } catch {
        $msg = $_.ErrorDetails.Message
        if ($msg -match "already exists") { Write-Host "GitHub repo already exists, pushing" }
        else { Write-Host "GitHub create failed: $msg" }
    }
    Set-RemoteUrl "github" "https://x-access-token:$GitHubToken@github.com/$GitHubOwner/$RepoName.git"
    & git push -u github main 2>&1 | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "github push failed" }
    git remote set-url github "https://github.com/$GitHubOwner/$RepoName.git"
    Write-Host "GitHub done: https://github.com/$GitHubOwner/$RepoName"
}

git remote -v