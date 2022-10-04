mount -uw /
mount -uw /private/preboot
sleep 1
launchctl bootstrap system /Library/LaunchDaemons
/etc/rc.d/*
sbreload
