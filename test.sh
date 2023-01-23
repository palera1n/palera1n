DONE=$(gum style --height 5 --width 25 --padding '1 3' --border double --border-foreground 57  "Done!" "The device should now boot to $(gum style --foreground 212 "iOS")")
UNLOCK=$(gum style --width 25 --padding '1 3' --border double --border-foreground 212 "When you unlock the device, it will respring about $(gum style --foreground "#04B575" "30 seconds") later.")
FIRST=$(gum style  --height 5 --width 35 --padding '1 8' --border double --border-foreground 255 "If this is your first time jailbreaking," "open the new palera1n app, then press $(gum style --foreground 57 "Install").")
ISSUE=$(gum style  --height 7 --width 35 --padding '1 5' --border double --border-foreground 120  "If you have any issues, please first check the $(gum style --foreground 212 "common-issues.md") document for common issues")
DISCORD=$(gum style  --height 7 --width 35 --padding '1 5' --border double --border-foreground 120  "If that list doesn't solve your issue, join the $(gum style --foreground 212 "Discord") server" "and ask for help:" "$(gum style --foreground 212 "https://dsc.gg/palera1n")")
ENJOY=$(gum style --width 15 --align center --padding "1 1" --border double --border-foreground 57 $(gum style --foreground 212 "Enjoy!"))

DONE_UNLOCK=$(gum join "$DONE" "$UNLOCK")
FIRST_ISSUE=$(gum join "$FIRST" "$ISSUE")
DISCORD_ENJOY=$(gum join "$DISCORD" "$ENJOY")
gum join --align center --vertical "$DONE_UNLOCK" "$FIRST_ISSUE" "$DISCORD_ENJOY"
