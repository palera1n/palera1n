#!/usr/bin/env bash

set -e

# Variables
version="1.0.0"
os=$(uname)
dir="$(pwd)/binaries/$os"
if [[ "$@" == *"--debug"* ]]; then
    out=/dev/stdout
else
    out=/dev/null
fi

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
    rm -rf boot-* work
    echo "[*] Removed the created boot files"
    exit
fi

# Download gaster
if [ ! -e "$dir"/gaster ]; then
    curl -sLO https://nightly.link/verygenericname/gaster/workflows/makefile/main/gaster-"$os".zip
    unzip gaster-"$os".zip >> /dev/null
    mv gaster "$dir"/
    rm -rf gaster gaster-"$os".zip
fi

# Check for pyimg4
if ! python3 -c 'import pkgutil; exit(not pkgutil.find_loader("pyimg4"))'; then
    echo '[-] pyimg4 not installed. Press any key to install it, or press ctrl + c to cancel'
    read -n 1 -s
    python3 -m pip install pyimg4 > "$out"
fi

# Re-create work dir if it exists, else, make it
if [ -e work ]; then
    rm -rf work
    mkdir work
else
    mkdir work
fi

chmod +x "$dir"/*

echo "palera1n | Version $version"
echo "Written by Nebula | Some code by Nathan | Patching commands and ramdisk by Mineek | Loader app by Amy"
echo ""

# Get device's iOS version from ideviceinfo if in normal mode
if [ "$2" = '--dfu' ]; then
    if [ -z "$3" ]; then
        echo "[-] When using --dfu, please pass the version you're device is on"
        exit
    else
        version=$3
    fi
else
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep 'Manufacturer: Apple Inc.' >> /dev/null); then
            echo "[*] Waiting for device in normal mode"
        fi

        while ! (system_profiler SPUSBDataType 2> /dev/null | grep 'Manufacturer: Apple Inc.' >> /dev/null); do
            sleep 1
        done

        defaults write -g ignore-devices -bool true
        defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool true
        killall Finder
    else
        if ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); then
            echo "[*] Waiting for device in normal mode"
        fi

        while ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); do
            sleep 1
        done
    fi
    version=$(ideviceinfo | grep "ProductVersion: " | sed 's/ProductVersion: //')
    arch=$(ideviceinfo | grep "CPUArchitecture: " | sed 's/CPUArchitecture: //')
    if [ ! "$arch" = "arm64" ]; then
        echo "[-] palera1n doesn't, and never will, work on non-checkm8 devices"
        exit
    fi
    echo "Hello, $(ideviceinfo | grep "ProductType: " | sed 's/ProductType: //') on $version!"
fi

# Put device into recovery mode, and set auto-boot to true
if [ ! "$2" = '--dfu' ]; then
    echo "[*] Switching device into recovery mode..."
    ideviceenterrecovery $(ideviceinfo | grep "UniqueDeviceID: " | sed 's/UniqueDeviceID: //') > "$out"
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); then
            echo "[*] Waiting for device to reconnect in recovery mode"
        fi

        while ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); do
            sleep 1
        done
    else
        if ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); then
            echo "[*] Waiting for device to reconnect in recovery mode"
        fi

        while ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); do
            sleep 1
        done
    fi
    "$dir"/irecovery -c "setenv auto-boot true"
    "$dir"/irecovery -c "saveenv"
fi

# Grab more info
echo "[*] Getting device info..."
cpid=$("$dir"/irecovery -q | grep CPID | sed 's/CPID: //')
model=$("$dir"/irecovery -q | grep MODEL | sed 's/MODEL: //')
deviceid=$("$dir"/irecovery -q | grep PRODUCT | sed 's/PRODUCT: //')
ipswurl=$(curl -sL "https://api.ipsw.me/v4/device/$deviceid?type=ipsw" | "$dir"/jq '.firmwares | .[] | select(.version=="'"$version"'") | .url' --raw-output)

# Have the user put the device into DFU
if [ ! "$2" = '--dfu' ]; then
    echo "[*] Press any key when ready for DFU mode"
    read -n 1 -s
    step 3 "Get ready"
    step 4 "Hold volume down + side button" &
    sleep 3
    "$dir"/irecovery -c "reset"
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

if [ ! -e boot-"$deviceid" ]; then
    sleep 2
    echo "[*] Pwning device"
    "$dir"/gaster pwn > "$out"
    sleep 1

    # Downloading files, and decrypting iBSS/iBEC
    mkdir boot-"$deviceid"
    cd work

    echo "[*] Downloading BuildManifest"
    "$dir"/pzb -g BuildManifest.plist "$ipswurl" > "$out"
    "$dir"/img4tool -e -s "$1" -m IM4M > "$out"

    echo "[*] Downloading and decrypting iBSS"
    "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl" > "$out"
    "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBSS.dec > "$out"

    echo "[*] Downloading and decrypting iBEC"
    "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBEC[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl" > "$out"
    "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBEC[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBEC.dec > "$out"

    echo "[*] Downloading DeviceTree"
    #$dir/pzb -g "$(awk "/""$cpid""/{x=1}x&&/DeviceTree[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" $ipswurl > "$out"
    "$dir"/pzb -g Firmware/all_flash/DeviceTree."$model".im4p "$ipswurl" > "$out"

    echo "[*] Downloading trustcache"
    if [ "$os" = 'Darwin' ]; then
       "$dir"/pzb -g "$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."StaticTrustCache"."Info"."Path" xml1 -o - BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)" "$ipswurl" > "$out"
    else
       "$dir"/pzb -g "$("$dir"/PlistBuddy BuildManifest.plist -c "Print BuildIdentities:0:Manifest:StaticTrustCache:Info:Path" | sed 's/"//g')" "$ipswurl" > "$out"
    fi

    #if [[ "$@" == *"install"* ]]; then
    #    echo "[*] Downloading ramdisk"
    #    if [ "$os" = 'Darwin' ]; then
    #        $dir/pzb -g "$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)" $ipswurl > "$out"
    #    else
    #        $dir/pzb -g "$($dir/PlistBuddy BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')" $ipswurl > "$out"
    #    fi
    #fi

    echo "[*] Downloading kernelcache"
    "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl" > "$out"

    echo "[*] Patching and repacking iBSS/iBEC"
    "$dir"/iBoot64Patcher iBSS.dec iBSS.patched > "$out"
    "$dir"/iBoot64Patcher iBEC.dec iBEC.patched -b '-v keepsyms=1 debug=0xfffffffe panic-wait-forever=1 wdt=-1' > "$out"
    #if [[ "$@" == *"install"* ]]; then
    #    $dir/iBoot64Patcher iBEC.patched restore_ibec.patched -b '-v rd=md0 debug=0x2014e wdt=-1' > "$out"
    #fi
    cd ..
    "$dir"/img4 -i work/iBSS.patched -o boot-"$deviceid"/iBSS.img4 -M work/IM4M -A -T ibss > "$out"
    "$dir"/img4 -i work/iBEC.patched -o boot-"$deviceid"/iBEC.img4 -M work/IM4M -A -T ibec > "$out"
    #if [[ "$@" == *"install"* ]]; then
    #    $dir/img4 -i work/restore_ibec.patched -o boot-"$deviceid"/restore_ibec.img4 -M work/IM4M -A -T ibec > "$out"
    #fi

    echo "[*] Patching and converting kernelcache"
    if [[ "$deviceid" == *'iPhone8'* ]]; then
        python3 -m pyimg4 im4p extract -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o work/kcache.raw --extra work/kpp.bin > "$out"
    else
        python3 -m pyimg4 im4p extract -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o work/kcache.raw > "$out"
    fi
    "$dir"/Kernel64Patcher work/kcache.raw work/kcache.patched -a -o > "$out"
    if [[ "$deviceid" == *'iPhone8'* ]]; then
        python3 -m pyimg4 im4p create -i work/kcache.patched -o work/krnlboot.im4p --extra work/kpp.bin -f rkrn --lzss > "$out"
    else
        python3 -m pyimg4 im4p create -i work/kcache.patched -o work/krnlboot.im4p -f rkrn --lzss > "$out"
    fi
    python3 -m pyimg4 img4 create -p work/krnlboot.im4p -o boot-"$deviceid"/kernelcache.img4 -m work/IM4M > "$out"

    echo "[*] Converting DeviceTree"
    "$dir"/img4 -i work/"$(awk "/""$model""/{x=1}x&&/DeviceTree[.]/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]all_flash[/]//')" -o boot-"$deviceid"/devicetree.img4 -M work/IM4M -T rdtr > "$out"

    echo "[*] Patching and converting trustcache"
    if [ "$os" = 'Darwin' ]; then
        "$dir"/img4 -i work/"$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."StaticTrustCache"."Info"."Path" xml1 -o - work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1 | sed 's/Firmware\///')" -o boot-"$deviceid"/trustcache.img4 -M work/IM4M -T rtsc > "$out"
    else
        "$dir"/img4 -i work/"$("$dir"/PlistBuddy work/BuildManifest.plist -c "Print BuildIdentities:0:Manifest:StaticTrustCache:Info:Path" | sed 's/"//g'| sed 's/Firmware\///')" -o boot-"$deviceid"/trustcache.img4 -M work/IM4M -T rtsc > "$out"
    fi

    #if [[ "$@" == *"install"* ]]; then
    #    echo "[*] Making ramdisk... this may take awhile"
    #    if [ "$os" = 'Darwin' ]; then
    #        $dir/img4 -i work/"$(/usr/bin/plutil -extract "BuildIdentities".0."Manifest"."RestoreRamDisk"."Info"."Path" xml1 -o - work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | head -1)" -o work/ramdisk.dmg > "$out"
    #    else
    #        $dir/img4 -i work/"$(Linux/PlistBuddy work/BuildManifest.plist -c "Print BuildIdentities:0:Manifest:RestoreRamDisk:Info:Path" | sed 's/"//g')" -o work/ramdisk.dmg > "$out"
    #    fi
    #    $dir/hfsplus work/ramdisk.dmg grow 300000000 > "$out"
    #    $dir/hfsplus work/ramdisk.dmg untar other/ramdisk.tar.gz > "$out"
    #    $dir/img4 -i work/ramdisk.dmg -o boot-"$deviceid"/ramdisk.img4 -M work/IM4M -A -T rdsk > "$out"
    #fi
fi

echo "[*] Pwning device"
sleep 2
"$dir"/gaster pwn > "$out"
sleep 2
"$dir"/gaster reset > "$out"
sleep 3
echo "[*] Booting device"
"$dir"/irecovery -f boot-"$deviceid"/iBSS.img4
sleep 3
#if [[ "$@" == *"install"* ]]; then
#    $dir/irecovery -f boot-"$deviceid"/restore_ibec.img4
#    sleep 2
#else
"$dir"/irecovery -f boot-"$deviceid"/iBEC.img4
sleep 2
#fi
if [[ "$cpid" == *"0x80"* ]]; then
    #if [[ "$@" == *"install"* ]]; then
    #    $dir/irecovery -f boot-"$deviceid"/restore_ibec.img4
    #else
    #    $dir/irecovery -f boot-"$deviceid"/iBEC.img4
    #fi
    sleep 2
    "$dir"/irecovery -c "go"
    sleep 3
fi
#if [[ "$@" == *"install"* ]]; then
#    $dir/irecovery -f boot-"$deviceid"/ramdisk.img4
#    sleep 2
#    $dir/irecovery -c "ramdisk"
#    sleep 2
#fi
"$dir"/irecovery -f boot-"$deviceid"/devicetree.img4
sleep 1
"$dir"/irecovery -c "devicetree"
sleep 1
"$dir"/irecovery -f boot-"$deviceid"/trustcache.img4
sleep 1
"$dir"/irecovery -c "firmware"
sleep 1
"$dir"/irecovery -f boot-"$deviceid"/kernelcache.img4
sleep 2
"$dir"/irecovery -c "bootx"

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
#if [[ "$@" == *"install"* ]]; then
#    echo "The device should now reboot after about 30 seconds, then you can rerun the script without the install arg"
#else
echo "The device should now boot to iOS"
echo "If you already have installed Pogo, click uicache and remount preboot in the tools section"
echo "If not, get an IPA from the latest action build of Pogo and install with TrollStore"
echo "Add the repo mineek.github.io/repo for Procursus"
#fi
