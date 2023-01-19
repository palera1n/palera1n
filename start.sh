#!/usr/bin/env bash
MODE=$(gum table < modes.csv | cut -d ',' -f 1)
IOS=$(gum input --placeholder 'iOS version please... (You may skip)')
echo 'Other flags?'
FLAGS=$(cat flags.md | gum choose --no-limit)
SHORT_FLAGS=$(echo $SUIT | cut -d' ' -f2 | tr -d '()')
./palera1n.sh $IOS $MODE $SHORT_FLAGS
