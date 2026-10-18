[CmdletBinding()]
param(
    [ValidateSet('sonic-adventure-2-battle')]
    [string]$Project = 'sonic-adventure-2-battle',
    [Parameter(Mandatory = $true)]
    [string]$Iso,
    [ValidateSet('NTSC-U', 'PAL')]
    [string]$Region = 'NTSC-U',
    [string]$ExpectedSha256,
    [string]$WslDistro = 'Ubuntu-24.04'
)

$ErrorActionPreference = 'Stop'
$pythonRuntime = Get-Command python -ErrorAction SilentlyContinue
if (-not $pythonRuntime) {
    throw 'Python 3.10 oder neuer wird benoetigt.'
}
$prepareArguments = @((Join-Path $PSScriptRoot 'tools/prepare_region.py'), $Project,
    '--iso', $Iso, '--region', $Region, '--wsl-distro', $WslDistro)
if ($ExpectedSha256) { $prepareArguments += @('--expected-sha256', $ExpectedSha256) }
& $pythonRuntime.Source @prepareArguments
exit $LASTEXITCODE
