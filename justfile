shebang := if os() == 'windows' { 'pwsh.exe' } else { '/usr/bin/env pwsh' }
set shell := ["pwsh", "-c"]
set windows-shell := ["pwsh.exe", "-NoLogo", "-Command"]
set dotenv-load := true
# INFO: really dont want to meddle with the .env, direnv is also related to this.
# WARN: should have get them in .gitignore.
set dotenv-filename	:= ".env"
# set dotenv-required := true
export JUST_ENV := "just_env" # WARN: this is also a method to export env var.
help:
    @just --list -f "{{home_directory()}}/justfile"

default_arg := 'TODO:'
alias td := todo
todo todo_arg=default_arg:
    rg {{todo_arg}} -g '!justfile' -g "!third_party"

wt:
    #!{{ shebang }}
    wt -f new-tab -p "Developer PowerShell for VS 2022" -d  {{ invocation_directory() }} --tabColor '#FFFFFF'

alias b := build
build:
    cd build && ninja all

alias fmt := format
format args="nothing":
    # format something as `biome format --write`

# INFO: basic `run` recipe.
alias r := run
default_args := 'args here'
run args=default_args:
    @Write-Host {{default_args}} -ForegroundColor Red

var_test := "test format"
alias t := test
test *args:
   {{ if args == "" { "cd build && ninja test" } else { "cd build && ninja tests/" + args +"/test"} }}

alias cl := clean
clean:
    cd build && ninja clean

