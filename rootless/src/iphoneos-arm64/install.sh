#!/var/pkg/bin/bash

function clean() {
rm -f bootstrap-ssh-iphoneos-arm64.tar
rm -f org.coolstar.sileonightly_2.4_iphoneos-arm64.deb
rm -f install.sh
}

if stat /var/jb >/dev/null 2>&1; then
echo 'Already installed?'
clean
exit
fi

cd /var/root
mount -uw /private/preboot
mkdir /private/preboot/tempdir
tar --preserve-permissions -xkf bootstrap-ssh-iphoneos-arm64.tar -C /private/preboot/tempdir
mv -v /private/preboot/tempdir/var/jb /private/preboot/$(cat /private/preboot/active)/procursus
rm -rf /private/preboot/tempdir

ln -s /private/preboot/$(cat /private/preboot/active)/procursus /var/jb

/var/jb/prep_bootstrap.sh
/var/jb/usr/libexec/firmware

source /var/jb/etc/profile

echo "Installing Sileo-Nightly and upgrading Procursus packages..."
dpkg -i org.coolstar.sileonightly_2.4_iphoneos-arm64.deb > /dev/null
uicache -p /var/jb/Applications/Sileo-Nightly.app

echo "Installing ElleKit..."
dpkg --force-all -i ellekit_rootless.deb > /dev/null
ln -s /private/preboot/$(cat /private/preboot/active)/procursus/usr/lib/libsubstitute.dylib /private/preboot/$(cat /private/preboot/active)/procursus/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate

echo "Installing PreferenceLoader..."
dpkg -i preferenceloader_2.2.6-1debug_iphoneos-arm64.deb > /dev/null

apt-get update -o Acquire::AllowInsecureRepositories=true
apt-get dist-upgrade -y --allow-downgrades --allow-unauthenticated

echo "deb https://mineek.github.io/rootlessrepo/ ./" > /var/jb/etc/apt/sources.list.d/mineek.list

clean

echo "Done!"
