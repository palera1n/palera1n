#!/usr/bin/env bash
export LC_CTYPE="en_US.UTF-8"
MODE=$(gum table < modes.csv | cut -d ',' -f 1)
IOS=$(gum input --placeholder 'iOS version please... (optional)')
echo 'Other flags?'
FLAGS=$(gum choose --no-limit < flags.md)
SHORT_FLAGS=$(echo $FLAGS | cut -d ' ' -f2 | tr -d '()')
./palera1n.sh $IOS $MODE $SHORT_FLAGS
