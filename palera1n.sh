#!/usr/bin/env bash

set -e

# Variables
version="1.0.0"
os=$(uname)
dir="$(pwd)/binaries/$os"

# Functions
step() {
    for i in $(seq "$1" -1 1); do
        printf '\r\e[1;36m%s (%d) ' "$2" "$i"
        sleep 1
    done
    printf '\r\e[0m%s (0)\n' "$2"
}

# Error handler
ERR_HANDLER () {
    [ $? -eq 0 ] && exit
    echo "[-] An error occurred"
    if [ "$os" = 'Darwin' ]; then
        if [ ! "$2" = '--dfu' ]; then
            defaults write -g ignore-devices -bool false
            defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool false
            killall Finder
        fi
    fi
}
trap ERR_HANDLER EXIT

if [ "$1" = 'clean' ]; then
    rm -rf boot work
    echo "[*] Removed the created boot files"
    exit
fi

# Download gaster
if [ ! -e $dir/gaster ]; then
    curl -sLO https://nightly.link/verygenericname/gaster/workflows/makefile/main/gaster-$os.zip
    unzip gaster-$os.zip >> /dev/null
    mv gaster $dir/
    rm -rf gaster gaster-$os.zip
fi

# Re-create work dir if it exists, else, make it
if [ -e work ]; then
    rm -rf work
    mkdir work
else
    mkdir work
fi

chmod +x $dir/*

echo "palera1n | Version $version"
echo "Written by Nebula | Some code by Nathan | Patching commands and ramdisk by Mineek | Loader app by Amy"
echo ""

# Get device's iOS version from ideviceinfo if in normal mode
echo "[*] Getting device version..."
if [ "$2" = '--dfu' ]; then
    if [ -z "$3" ]; then
        echo "[-] When using --dfu, please pass the version you're device is on"
        exit
    else
        version=$3
    fi
else
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' iPhone:' >> /dev/null); then
            echo "[*] Waiting for device in normal mode"
        fi

        while ! (system_profiler SPUSBDataType 2> /dev/null | grep ' iPhone:' >> /dev/null); do
            sleep 1
        done

        defaults write -g ignore-devices -bool true
        defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool true
        killall Finder
    else
        if ! (lsusb 2> /dev/null | grep ' iPhone:' >> /dev/null); then
            echo "[*] Waiting for device in normal mode"
        fi

        while ! (lsusb 2> /dev/null | grep ' iPhone:' >> /dev/null); do
            sleep 1
        done
    fi
    version=$(ideviceinfo | grep "ProductVersion: " | sed 's/ProductVersion: //')
fi

# Put device into recovery mode, and set auto-boot to true
if [ ! "$2" = '--dfu' ]; then
    echo "[*] Switching device into recovery mode..."
    ideviceenterrecovery $(ideviceinfo | grep "UniqueDeviceID: " | sed 's/UniqueDeviceID: //') > /dev/null
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); then
            echo "[*] Waiting for device to reconnect in recovery mode"
        fi

        while ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); do
            sleep 1
        done
    else
        if ! (lsusb 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); then
            echo "[*] Waiting for device to reconnect in recovery mode"
        fi

        while ! (lsusb 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); do
            sleep 1
        done
    fi
    $dir/irecovery -c "setenv auto-boot true"
    $dir/irecovery -c "saveenv"
fi

# Grab more info
echo "[*] Getting device info..."
cpid=$($dir/irecovery -q | grep CPID | sed 's/CPID: //')
model=$($dir/irecovery -q | grep MODEL | sed 's/MODEL: //')
deviceid=$($dir/irecovery -q | grep PRODUCT | sed 's/PRODUCT: //')
ipswurl=$(curl -sL "https://api.ipsw.me/v4/device/$deviceid?type=ipsw" | $dir/jq '.firmwares | .[] | select(.version=="'$version'") | .url' --raw-output)

# Have the user put the device into DFU
if [ ! "$2" = '--dfu' ]; then
    echo "[*] Press any key when ready for DFU mode"
    read -n 1 -s
    step 3 "Get ready"
    step 4 "Hold volume down + side button" &
    sleep 3
    irecovery -c reset
    step 1 "Keep holding"
    step 10 'Release side button, but keep holding volume down'
    sleep 1
fi

# Check if device entered dfu
if [ ! "$2" = '--dfu' ]; then
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (DFU Mode):' >> /dev/null); then
            echo "[-] Device didn't go in DFU mode, please rerun the script and try again"
            exit
        fi
    else
        if ! (lsusb 2> /dev/null | grep ' Apple Mobile Device (DFU Mode):' >> /dev/null); then
            echo "[-] Device didn't go in DFU mode, please rerun the script and try again"
            exit
        fi
    fi
    echo "[*] Device entered DFU!"
fi

sleep 2
$dir/gaster pwn > /dev/null

if [ ! -e boot ]; then
    # Downloading files, and decrypting iBSS/iBEC
    mkdir boot
    cd work

    echo "[*] Downloading BuildManifest"
    $dir/pzb -g BuildManifest.plist $ipswurl > /dev/null
    $dir/img4tool -e -s $1 -m IM4M > /dev/null

    echo "[*] Downloading and decrypting iBSS"
    $dir/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" $ipswurl > /dev/null
    $dir/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBSS.dec > /dev/null

    echo "[*] Downloading and decrypting iBEC"
    $dir/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBEC[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" $ipswurl > /dev/null
    $dir/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBEC[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBEC.dec > /dev/null

    echo "[*] Downloading DeviceTree"
    $dir/pzb -g "$(awk "/""$cpid""/{x=1}x&&/DeviceTree[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" $ipswurl > /dev/null

    echo "[*] Downloading trustcache"
    if [ "$os" = 'Darwin' ]; then
        $dir/pzb -g Firmware/"$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)".trustcache $ipswurl > /dev/null
    else
        $dir/pzb -g Firmware/"$($dir/PlistBuddy BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')".trustcache $ipswurl > /dev/null
    fi

    if [[ "$@" == *"install"* ]]; then
        echo "[*] Downloading ramdisk"
        if [ "$os" = 'Darwin' ]; then
            $dir/pzb -g "$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)" $ipswurl > /dev/null
        else
            $dir/pzb -g "$($dir/PlistBuddy BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')" $ipswurl > /dev/null
        fi
    fi

    echo "[*] Downloading kernelcache"
    $dir/pzb -g "$(awk "/""$cpid""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" $ipswurl > /dev/null

    echo "[*] Patching and repacking iBSS/iBEC"
    $dir/iBoot64Patcher iBSS.dec iBSS.patched > /dev/null
    $dir/iBoot64Patcher iBEC.dec iBEC.patched -b '-v keepsyms=1 debug=0xfffffffe panic-wait-forever=1' #> /dev/null
    if [[ "$@" == *"install"* ]]; then
        $dir/iBoot64Patcher iBEC.patched restore_ibec.patched -b 'rd=md0 debug=0x2014e -v wdt=-1' > /dev/null
    fi
    cd ..
    $dir/img4 -i work/iBSS.patched -o boot/iBSS.img4 -M work/IM4M -A -T ibss > /dev/null
    $dir/img4 -i work/iBEC.patched -o boot/iBEC.img4 -M work/IM4M -A -T ibec > /dev/null
    if [[ "$@" == *"install"* ]]; then
        $dir/img4 -i work/restore_ibec.patched -o boot/restore_ibec.img4 -M work/IM4M -A -T ibec > /dev/null
    fi

    echo "[*] Patching and converting kernelcache"
    $dir/img4 -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o work/kcache.raw > /dev/null
    $dir/Kernel64Patcher work/kcache.raw work/kcache.patched -a -o > /dev/null
    python3 kerneldiff.py work/kcache.raw work/kcache.patched work/kc.bpatch > /dev/null
    $dir/img4 -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o boot/kernelcache.img4 -M work/IM4M -T rkrn -P work/kc.bpatch `if [ "$os" = 'Linux' ]; then echo "-J"; fi` > /dev/null

    echo "[*] Converting DeviceTree"
    $dir/img4 -i work/"$(awk "/""$model""/{x=1}x&&/DeviceTree[.]/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]all_flash[/]//')" -o boot/devicetree.img4 -M work/IM4M -T rdtr > /dev/null

    echo "[*] Patching and converting trustcache"
    if [ "$os" = 'Darwin' ]; then
        $dir/img4 -i work/"$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)".trustcache -o boot/trustcache.img4 -M work/IM4M -T rtsc > /dev/null
    else
        $dir/img4 -i work/"$(Linux/PlistBuddy work/BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')".trustcache -o boot/trustcache.img4 -M work/IM4M -T rtsc > /dev/null
    fi

    if [[ "$@" == *"install"* ]]; then
        echo "[*] Making ramdisk... this may take awhile"
        if [ "$os" = 'Darwin' ]; then
            $dir/img4 -i work/"$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)" -o work/ramdisk.dmg > /dev/null
        else
            $dir/img4 -i work/"$(Linux/PlistBuddy work/BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')" -o work/ramdisk.dmg > /dev/null
        fi
        $dir/hfsplus work/ramdisk.dmg grow 300000000 > /dev/null
        $dir/hfsplus work/ramdisk.dmg untar installer/ramdisk.tar.gz > /dev/null
        $dir/img4 -i work/ramdisk.dmg -o boot/ramdisk.img4 -M work/IM4M -A -T rdsk > /dev/null
    fi
fi

echo "[*] Booting device"
$dir/irecovery -f boot/iBSS.img4
sleep 2
$dir/irecovery -f boot/iBSS.img4
sleep 3
if [[ "$@" == *"install"* ]]; then
    $dir/irecovery -f boot/restore_ibec.img4
    sleep 2
else
    $dir/irecovery -f boot/iBEC.img4
    sleep 2
fi
if [[ "$cpid" == *"0x80"* ]]; then
    if [[ "$@" == *"install"* ]]; then
        $dir/irecovery -f boot/restore_ibec.img4
    else
        $dir/irecovery -f boot/iBEC.img4
    fi
    sleep 2
    $dir/irecovery -c "go"
    sleep 5
fi
if [[ "$@" == *"install"* ]]; then
    $dir/irecovery -f boot/ramdisk.img4
    sleep 2
    $dir/irecovery -c "ramdisk"
    sleep 2
fi
$dir/irecovery -f boot/devicetree.img4
sleep 1
$dir/irecovery -c "devicetree"
sleep 1
$dir/irecovery -f boot/trustcache.img4
sleep 1
$dir/irecovery -c "firmware"
sleep 1
$dir/irecovery -f boot/kernelcache.img4
sleep 1
$dir/irecovery -c "bootx"

if [ "$os" = 'Darwin' ]; then
    if [ ! "$2" = '--dfu' ]; then
        defaults write -g ignore-devices -bool false
        defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool false
        killall Finder
    fi
fi

rm -rf work
echo ""
echo "Done!"
echo "The device should now boot to iOS, and you can install Pogo with TrollStore"
