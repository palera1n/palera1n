#!/usr/bin/env bash
export LC_CTYPE="en_US.UTF-8"
export GUM_SPIN_SHOW_OUTPUT='true'
MODE=$(gum table -f gum/modes.csv -w 24,23,65 --height="15" --header.foreground='222' --cell.border='rounded' --cell.foreground='222' | cut -d ',' -f 1)
IOS=$(gum input --placeholder 'optional' --prompt.foreground "212" --prompt "Whats your iOS version? > ")
echo 'Other flags?'
FLAGS=$(gum choose --cursor-prefix "[ ] " --selected-prefix "[✓] " --no-limit --item.foreground='222' < gum/flags.md | cut -d ' ' -f3 | tr -d '()')
./palera1n.sh $IOS $MODE $FLAGS
