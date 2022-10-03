mount -o rw /
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026161261077090365/bootstrap-ssh.tar
tar --preserve-permissions -xkf bootstrap-ssh.tar -C /
/prep_bootstrap.sh
launchctl unload /Library/LaunchDaemons/com.openssh.sshd.plist && launchctl load /Library/LaunchDaemons/com.openssh.sshd.plist
apt update
apt upgrade -y
apt install curl -y
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026162195517681788/com.ex.substitute_2.2.0_iphoneos-arm.deb
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026162195794497547/com.saurik.substrate.safemode_0.9.6005_iphoneos-arm.deb
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026162196104880191/org.coolstar.sileo_2.3_iphoneos-arm.deb
dpkg -i *.deb