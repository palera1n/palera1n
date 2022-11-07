mount -uw /
mount -uw /private/preboot
sleep 1
nvram auto-boot=false
launchctl bootstrap system /Library/LaunchDaemons
/etc/rc.d/*
sleep 1
/usr/bin/launchctl reboot userspace
