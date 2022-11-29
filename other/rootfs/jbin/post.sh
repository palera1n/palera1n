binpack=/jbin/binpack

# wait for springboard to load
$binpack/bin/sleep 15
while ! (/bin/ps -auroot | $binpack/usr/bin/grep SpringBoard 2> /dev/null > /dev/null); do
    $binpack/bin/sleep 1
done

# wait to be safe
$binpack/bin/sleep 15

# uicache loader app
$binpack/bin/mkdir -p /var/.palera1n
$binpack/bin/rm -rf /var/.palera1n/loader.app
$binpack/bin/cp -R /jbin/loader.app /var/.palera1n
$binpack/usr/bin/uicache -p /var/.palera1n/loader.app

# link dyld if using semi tethered
if [ ! -d "/System/Library/Caches/com.apple.dyld" ]; then
    $binpack/bin/ln -s /System/Cryptexes/OS/System/Library/Caches/com.apple.dyld /System/Library/Caches/
fi

echo "[post.sh] done"
exit
