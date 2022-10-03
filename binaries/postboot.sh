mount -o rw /
sleep 1
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026162195517681788/com.ex.substitute_2.2.0_iphoneos-arm.deb
curl -sLO https://cdn.discordapp.com/attachments/1017153024768081921/1026162195794497547/com.saurik.substrate.safemode_0.9.6005_iphoneos-arm.deb
dpkg -i *.deb
sbreload