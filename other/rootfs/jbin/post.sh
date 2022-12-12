binpack=/jbin/binpack

# uicache loader app
$binpack/bin/mkdir -p /var/.palera1n
$binpack/bin/rm -rf /var/.palera1n/loader.app
$binpack/bin/cp -R /jbin/loader.app /var/.palera1n
$binpack/usr/bin/uicache -p /var/.palera1n/loader.app

# link dyld if using semi tethered (iOS 16)
if [ -d "/System/Cryptexes/OS/System/Library/Caches/com.apple.dyld" ]; then
    if [ ! -d "/System/Library/Caches/com.apple.dyld" ]; then
        $binpack/bin/ln -s /System/Cryptexes/OS/System/Library/Caches/com.apple.dyld /System/Library/Caches/
    fi
fi

# respring
$binpack/usr/bin/killall -9 SpringBoard

echo "[post.sh] done"
exit
