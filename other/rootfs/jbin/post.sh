binpack=/jbin/binpack

# uicache loader app
$binpack/bin/mkdir -p /var/.palera1n
$binpack/bin/rm -rf /var/.palera1n/loader.app
$binpack/bin/cp -R /jbin/loader.app /var/.palera1n
$binpack/usr/bin/uicache -p /var/.palera1n/loader.app

# respring
$binpack/usr/bin/killall -9 SpringBoard

echo "[post.sh] done"
exit
