#!/usr/bin/env bash
MODE=$(gum table < others/modes.csv | cut -d ',' -f 1)
IOS=$(gum input --placeholder 'iOS version please... (optional)')
echo 'Other flags?'
FLAGS=$(gum choose --no-limit < other/flags.md)
SHORT_FLAGS=$(echo $FLAGS | cut -d ' ' -f2 | tr -d '()')
./main.sh $IOS $MODE $SHORT_FLAGS
