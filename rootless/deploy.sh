#!/bin/sh

if ! iproxy -v >/dev/null 2>&1; then
echo 'iproxy not installed'
exit
fi

echo "Downloading resources..."
IPROXY=$(iproxy 28605 44 >/dev/null 2>&1 & echo $!)
curl -sLOOOOO https://apt.procurs.us/bootstraps/1900/bootstrap-ssh-iphoneos-arm64.tar.zst \
https://raw.githubusercontent.com/elihwyma/Pogo/1724d2864ca55bc598fa96bee62acad875fe5990/Pogo/Required/org.coolstar.sileonightly_2.4_iphoneos-arm64.deb \
https://cdn.discordapp.com/attachments/1028398976640229380/1058815457508995082/ellekit_0.1.31.gce07f57.dirty_iphoneos-arm64_2022-12-31.deb \
https://cdn.discordapp.com/attachments/1028398976640229380/1056844445892481074/preferenceloader_2.2.6-1debug_iphoneos-arm64.deb

unzstd bootstrap-ssh-iphoneos-arm64.tar.zst
mv ellekit*.deb ellekit_rootless.deb

echo "Copying resources to your device..."
echo "Default password is: alpine"
if scp -O /dev/null /dev/zero 2>/dev/null; then
    scp -O -qP28605 -o "StrictHostKeyChecking no" -o "UserKnownHostsFile=/dev/null" bootstrap-ssh-iphoneos-arm64.tar \
    org.coolstar.sileonightly_2.4_iphoneos-arm64.deb \
    ellekit_rootless.deb \
    preferenceloader_2.2.6-1debug_iphoneos-arm64.deb \
    src/iphoneos-arm64/install.sh \
    root@127.0.0.1:/var/root/
else
    scp -qP28605 -o "StrictHostKeyChecking no" -o "UserKnownHostsFile=/dev/null" bootstrap-ssh-iphoneos-arm64.tar \
    org.coolstar.sileonightly_2.4_iphoneos-arm64.deb \
    ellekit_rootless.deb \
    preferenceloader_2.2.6-1debug_iphoneos-arm64.deb \
    src/iphoneos-arm64/install.sh \
    root@127.0.0.1:/var/root/
fi

echo "Bootstrapping your device..."
ssh -qp28605 -o "StrictHostKeyChecking no" -o "UserKnownHostsFile=/dev/null" root@127.0.0.1 "/var/pkg/bin/bash /var/root/install.sh"

rm -rf bootstrap-ssh-iphoneos-arm64.tar.zst
rm -rf bootstrap-ssh-iphoneos-arm64.tar
rm -rf org.coolstar.sileonightly_2.4_iphoneos-arm64.deb
rm -rf ellekit_rootless.deb
rm -rf preferenceloader_2.2.6-1debug_iphoneos-arm64.deb

kill "$IPROXY"


