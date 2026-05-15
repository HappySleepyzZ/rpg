param(
  [string]$WorkspacePath = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

$codexHome = Join-Path $env:USERPROFILE ".codex-openai"
$configPath = Join-Path $codexHome "config.toml"

New-Item -ItemType Directory -Force -Path $codexHome | Out-Null

if (-not (Test-Path -LiteralPath $configPath)) {
@'
# Separate Codex home for signing in with an OpenAI account.
# This intentionally does not copy auth.json or Paperhub provider settings.
model = "gpt-5.5"
model_reasoning_effort = "medium"

[windows]
sandbox = "unelevated"
'@ | Set-Content -LiteralPath $configPath -Encoding UTF8
}

$env:CODEX_HOME = $codexHome
& codex app $WorkspacePath
