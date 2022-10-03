mount -uw /
mount -uw /private/preboot
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026161261077090365/bootstrap-ssh.tar
tar --preserve-permissions -xkf bootstrap-ssh.tar -C /
/prep_bootstrap.sh
launchctl unload /Library/LaunchDaemons/com.openssh.sshd.plist && launchctl load /Library/LaunchDaemons/com.openssh.sshd.plist
uicache -u /var/jb/Applications/Sileo.app
rm -rf /var/jb
rm -rf /private/preboot/$(cat /private/preboot/active)/procursus
apt update
apt upgrade -y
apt install curl -y
curl -sLO https://apt.bingner.com/debs/1443.00/com.ex.substitute_2.2.3_iphoneos-arm.deb
curl -sLO https://apt.bingner.com/debs/1443.00/com.saurik.substrate.safemode_0.9.6005_iphoneos-arm.deb
curl -sLO https://github.com/coolstar/Odyssey-bootstrap/raw/master/org.swift.libswift_5.0-electra2_iphoneos-arm.deb
curl -sLO https://github.com/coolstar/Odyssey-bootstrap/raw/master/org.coolstar.sileo_2.3_iphoneos-arm.deb
dpkg -i *.deb
