mount -uw /
sleep 1
curl -sLO https://apt.bingner.com/debs/1443.00/com.ex.substitute_2.2.3_iphoneos-arm.deb
curl -sLO https://apt.bingner.com/debs/1443.00/com.saurik.substrate.safemode_0.9.6005_iphoneos-arm.deb
dpkg -i *.deb
sbreload
