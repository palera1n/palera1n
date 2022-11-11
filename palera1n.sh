#!/usr/bin/env bash

mkdir -p logs
set -e

{

echo "[*] Command ran:`if [ $EUID = 0 ]; then echo " sudo"; fi` ./palera1n.sh $@"

# =========
# Variables
# =========
ipsw="" # IF YOU WERE TOLD TO PUT A CUSTOM IPSW URL, PUT IT HERE. YOU CAN FIND THEM ON https://appledb.dev
version="1.2.0"
os=$(uname)
dir="$(pwd)/binaries/$os"
commit=$(git rev-parse --short HEAD)
branch=$(git rev-parse --abbrev-ref HEAD)

# =========
# Functions
# =========
step() {
    for i in $(seq "$1" -1 1); do
        printf '\r\e[1;36m%s (%d) ' "$2" "$i"
        sleep 1
    done
    printf '\r\e[0m%s (0)\n' "$2"
}

_wait() {
    if [ "$1" = 'normal' ]; then
        if [ "$os" = 'Darwin' ]; then
            if ! (system_profiler SPUSBDataType 2> /dev/null | grep 'Manufacturer: Apple Inc.' >> /dev/null); then
                echo "[*] Waiting for device in normal mode"
            fi

            while ! (system_profiler SPUSBDataType 2> /dev/null | grep 'Manufacturer: Apple Inc.' >> /dev/null); do
                sleep 1
            done
        else
            if ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); then
                echo "[*] Waiting for device in normal mode"
            fi

            while ! (lsusb 2> /dev/null | grep ' Apple, Inc.' >> /dev/null); do
                sleep 1
            done
        fi
    elif [ "$1" = 'recovery' ]; then
        if [ "$os" = 'Darwin' ]; then
            if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); then
                echo "[*] Waiting for device to reconnect in recovery mode"
            fi

            while ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (Recovery Mode):' >> /dev/null); do
                sleep 1
            done
        else
            if ! (lsusb 2> /dev/null | grep 'Recovery Mode' >> /dev/null); then
                echo "[*] Waiting for device to reconnect in recovery mode"
            fi

            while ! (lsusb 2> /dev/null | grep 'Recovery Mode' >> /dev/null); do
                sleep 1
            done
        fi

        if [ "$1" = "--tweaks" ]; then
            "$dir"/irecovery -c "setenv auto-boot false"
            "$dir"/irecovery -c "saveenv"
        else
            "$dir"/irecovery -c "setenv auto-boot true"
            "$dir"/irecovery -c "saveenv"
        fi

        if [[ "$@" == *"--semi-tethered"* ]]; then
            "$dir"/irecovery -c "setenv auto-boot true"
            "$dir"/irecovery -c "saveenv"
        fi
    fi
}

_check_dfu() {
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (DFU Mode):' >> /dev/null); then
            echo "[-] Device didn't go in DFU mode, please rerun the script and try again"
            exit
        fi
    else
        if ! (lsusb 2> /dev/null | grep 'DFU Mode' >> /dev/null); then
            echo "[-] Device didn't go in DFU mode, please rerun the script and try again"
            exit
        fi
    fi
}

_info() {
    if [ "$1" = 'recovery' ]; then
        echo $("$dir"/irecovery -q | grep "$2" | sed "s/$2: //")
    elif [ "$1" = 'normal' ]; then
        echo $("$dir"/ideviceinfo | grep "$2: " | sed "s/$2: //")
    fi
}

_pwn() {
    pwnd=$(_info recovery PWND)
    if [ "$pwnd" = "" ]; then
        echo "[*] Pwning device"
        "$dir"/gaster pwn
        sleep 2
        "$dir"/gaster reset
        sleep 1
    fi
}

_dfuhelper() {
    echo "[*] Press any key when ready for DFU mode"
    read -n 1 -s
    step 3 "Get ready"
    step 4 "Hold volume down + side button" &
    sleep 3
    "$dir"/irecovery -c "reset"
    step 1 "Keep holding"
    step 10 'Release side button, but keep holding volume down'
    sleep 1
    
    _check_dfu
    echo "[*] Device entered DFU!"
}

_kill_if_running() {
    if (pgrep -u root -xf "$1" &> /dev/null > /dev/null); then
        # yes, it's running as root. kill it
        sudo killall $1
    else
        if (pgrep -x "$1" &> /dev/null > /dev/null); then
            killall $1
        fi
    fi
}

_beta_url() {
    if [[ "$deviceid" == *"iPad"* ]]; then
        json=$(curl -s 'https://api.appledb.dev/ios/iPadOS;19B5060d.json')
    else
        json=$(curl -s 'https://api.appledb.dev/ios/iOS;19B5060d.json')
    fi

    sources=$(echo "$json" | $dir/jq -r '.sources')
    beta_url=$(echo "$sources" | $dir/jq -r --arg deviceid "$deviceid" '.[] | select(.type == "ota" and (.deviceMap | index($deviceid))) | .links[0].url')
    echo "$beta_url"
}

_exit_handler() {
    if [ "$os" = 'Darwin' ]; then
        if [ ! "$1" = '--dfu' ]; then
            defaults write -g ignore-devices -bool false
            defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool false
            killall Finder
        fi
    fi
    [ $? -eq 0 ] && exit
    echo "[-] An error occurred"

    cd logs
    for file in *.log; do
        mv "$file" FAIL_${file}
    done
    cd ..

    echo "[*] A failure log has been made. If you're going to make a GitHub issue, please attach the latest log."
}
trap _exit_handler EXIT

# ===========
# Fixes
# ===========

# Prevent Finder from complaning
if [ "$os" = 'Darwin' ]; then
    defaults write -g ignore-devices -bool true
    defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool true
    killall Finder
fi

# ===========
# Subcommands
# ===========

if [ "$1" = 'clean' ]; then
    rm -rf boot* work .tweaksinstalled
    echo "[*] Removed the created boot files"
    exit
elif [ "$1" = 'dfuhelper' ]; then
    echo "[*] Running DFU helper"
    _dfuhelper
    exit
elif [ "$1" = '--restorerootfs' ]; then
    echo "[*] Restoring rootfs..."
    "$dir"/irecovery -n
    sleep 2
    echo "[*] Done, your device will boot into iOS now."
    # clean the boot files bcs we don't need them anymore
    rm -rf boot-"$deviceid" work .tweaksinstalled
    exit
fi

# ============
# Dependencies
# ============

# Download gaster
if [ -e "$dir"/gaster ]; then
    "$dir"/gaster &> /dev/null > /dev/null | grep -q 'usb_timeout: 5' && rm "$dir"/gaster
fi

if [ ! -e "$dir"/gaster ]; then
    curl -sLO https://nightly.link/palera1n/gaster/workflows/makefile/main/gaster-"$os".zip
    unzip gaster-"$os".zip
    mv gaster "$dir"/
    rm -rf gaster gaster-"$os".zip
fi

# Check for pyimg4
if ! python3 -c 'import pkgutil; exit(not pkgutil.find_loader("pyimg4"))'; then
    echo '[-] pyimg4 not installed. Press any key to install it, or press ctrl + c to cancel'
    read -n 1 -s
    python3 -m pip install pyimg4
fi

# ============
# Prep
# ============

# Update submodules
git submodule update --init --recursive

# Re-create work dir if it exists, else, make it
if [ -e work ]; then
    rm -rf work
    mkdir work
else
    mkdir work
fi

chmod +x "$dir"/*
#if [ "$os" = 'Darwin' ]; then
#    xattr -d com.apple.quarantine "$dir"/*
#fi

# ============
# Start
# ============

echo "palera1n | Version $version-$branch-$commit"
echo "Written by Nebula and Mineek | Some code and ramdisk from Nathan | Loader app by Amy"
echo ""

if [ "$1" = '--tweaks' ]; then
    _check_dfu
fi

if [ "$1" = '--tweaks' ] && [ ! -e ".tweaksinstalled" ] && [ ! -e ".disclaimeragree" ]; then
    echo "!!! WARNING WARNING WARNING !!!"
    echo "This flag will add tweak support BUT WILL BE TETHERED."
    echo "THIS ALSO MEANS THAT YOU'LL NEED A PC EVERY TIME TO BOOT."
    echo "THIS ONLY WORKS ON 15.0-15.3.1"
    echo "DO NOT GET ANGRY AT US IF UR DEVICE IS BORKED, IT'S YOUR OWN FAULT AND WE WARNED YOU"
    echo "DO YOU UNDERSTAND? TYPE 'Yes, do as I say' TO CONTINUE"
    read -r answer
    if [ "$answer" = 'Yes, do as I say' ]; then
        echo "Are you REALLY sure? WE WARNED YOU!"
        echo "Type 'Yes, I am sure' to continue"
        read -r answer
        if [ "$answer" = 'Yes, I am sure' ]; then
            echo "[*] Enabling tweaks"
            tweaks=1
            touch .disclaimeragree
        else
            echo "[*] Disabling tweaks"
            tweaks=0
        fi
    else
        echo "[*] Disabling tweaks"
        tweaks=0
    fi
fi

# Get device's iOS version from ideviceinfo if in normal mode
if [ "$1" = '--dfu' ] || [ "$1" = '--tweaks' ]; then
    if [ -z "$2" ]; then
        echo "[-] When using --dfu, please pass the version you're device is on"
        exit
    else
        version=$2
    fi
else
    _wait normal
    version=$(_info normal ProductVersion)
    arch=$(_info normal CPUArchitecture)
    if [ "$arch" = "arm64e" ]; then
        echo "[-] palera1n doesn't, and never will, work on non-checkm8 devices"
        exit
    fi
    echo "Hello, $(_info normal ProductType) on $version!"

    echo "[*] Switching device into recovery mode..."
    "$dir"/ideviceenterrecovery $(_info normal UniqueDeviceID)
    _wait recovery
fi

# Grab more info
echo "[*] Getting device info..."
cpid=$(_info recovery CPID)
model=$(_info recovery MODEL)
deviceid=$(_info recovery PRODUCT)
if [ ! "$ipsw" = "" ]; then
    ipswurl=$ipsw
else
    ipswurl=$(curl -sL "https://api.ipsw.me/v4/device/$deviceid?type=ipsw" | "$dir"/jq '.firmwares | .[] | select(.version=="'"$version"'") | .url' --raw-output)
fi

# Have the user put the device into DFU
if [ ! "$1" = '--dfu' ] && [ ! "$1" = '--tweaks' ]; then
    _dfuhelper
fi
sleep 2

# ============
# Ramdisk
# ============

# Dump blobs, and install pogo if needed
if [ ! -f blobs/"$deviceid"-"$version".shsh2 ]; then
    mkdir -p blobs

    cd ramdisk
    chmod +x sshrd.sh
    echo "[*] Creating ramdisk"
    ./sshrd.sh 15.7 `if [ ! "$1" = '--tweaks' ]; then echo "rootless"; fi`

    echo "[*] Booting ramdisk"
    ./sshrd.sh boot
    cd ..
    # if known hosts file exists, remove it
    if [ -f ~/.ssh/known_hosts ]; then
        rm ~/.ssh/known_hosts
    fi

    # Execute the commands once the rd is booted
    if [ "$os" = 'Linux' ]; then
        sudo "$dir"/iproxy 2222 22 &
    else
        "$dir"/iproxy 2222 22 &
    fi

    if ! ("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "echo connected" &> /dev/null); then
        echo "[*] Waiting for the ramdisk to finish booting"
    fi

    while ! ("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "echo connected" &> /dev/null); do
        sleep 1
    done

    echo "[*] Dumping blobs and installing Pogo"
    sleep 1
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /dev/rdisk1" | dd of=dump.raw bs=256 count=$((0x4000)) 
    "$dir"/img4tool --convert -s blobs/"$deviceid"-"$version".shsh2 dump.raw
    rm dump.raw

    if [[ ! "$@" == *"--no-install"* ]]; then
        # if --semi-tethered is passed, do this
        if [[ "$@" == *"--semi-tethered"* ]]; then
            echo "[*] Creating fakefs, this may take a while (up to 10 minutes)"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/newfs_apfs -A -D -o role=r -v System /dev/disk0s1"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/mount_apfs /dev/disk0s1s1 /mnt1"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/mount_apfs /dev/disk0s1s8 /mnt3"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/mount_apfs /dev/disk0s1s6 /mnt6"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cp -a /mnt1/. /mnt3/"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/umount /mnt1"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/umount /mnt3"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/umount /mnt6"
            sleep 5
            echo "[*] fakefs created, continuing..."
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/bin/mount_filesystems"
        else
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/bin/mount_filesystems"
        fi
        sleep 1
        tipsdir=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/bin/find /mnt2/containers/Bundle/Application/ -name 'Tips.app'" 2> /dev/null)
        sleep 1
        if [ "$tipsdir" = "" ]; then
            echo "[!] Tips is not installed. Once your device reboots, install Tips from the App Store and retry"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/reboot"
            sleep 1
            _kill_if_running iproxy
            exit
        fi
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/cp -rf /usr/local/bin/loader.app/* $tipsdir"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/chown 33 $tipsdir/Tips"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/chmod 755 $tipsdir/Tips $tipsdir/PogoHelper"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/chown 0 $tipsdir/PogoHelper"
    fi

    if [[ $1 == *"--tweaks"* ]]; then
        # execute nvram boot-args="-v keepsyms=1 debug=0x2014e launchd_unsecure_cache=1 launchd_missing_exec_no_panic=1 amfi=0xff amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 amfi_allow_research=1 amfi_unrestrict_task_for_pid=1 amfi_unrestricted_local_signing=1 cs_enforcement_disable=1 pmap_cs_allow_modified_code_pages=1 pmap_cs_enforce_coretrust=0 pmap_cs_unrestrict_pmap_cs_disable=1 -unsafe_kernel_text dtrace_dof_mode=1 panic-wait-forever=1 -panic_notify cs_debug=1 PE_i_can_has_debugger=1"
        # "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram boot-args=\"-v keepsyms=1 debug=0x2014e launchd_unsecure_cache=1 launchd_missing_exec_no_panic=1 amfi=0xff amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 amfi_allow_research=1 amfi_unrestrict_task_for_pid=1 amfi_unrestricted_local_signing=1 cs_enforcement_disable=1 pmap_cs_allow_modified_code_pages=1 pmap_cs_enforce_coretrust=0 pmap_cs_unrestrict_pmap_cs_disable=1 -unsafe_kernel_text dtrace_dof_mode=1 panic-wait-forever=1 -panic_notify cs_debug=1 PE_i_can_has_debugger=1\""
        # execute nvram allow-root-hash-mismatch=1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram allow-root-hash-mismatch=1"
        # execute nvram root-live-fs=1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram root-live-fs=1"
        # execute nvram auto-boot=false
        if [[ ! "$@" == *"--semi-tethered"* ]]; then
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram auto-boot=false"
        fi
        
        cd work
        echo "[*] Downloading BuildManifest"
        ipswurl=$(_beta_url)
        "$dir"/pzb -g AssetData/boot/BuildManifest.plist "$ipswurl"

        echo "[*] Getting apticket.der from device"
        has_active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "ls /mnt6/active" 2> /dev/null)
        if [ ! "$has_active" = "/mnt6/active" ]; then
            echo "[!] Active file does not exist! Please use SSH to create it"
            echo "    /mnt6/active should contain the name of the UUID in /mnt6"
            echo "    When done, type reboot in the SSH session, then rerun the script"
            echo "    ssh root@localhost -p 2222"
            exit
        fi
        active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /mnt6/active" 2> /dev/null)
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /mnt6/$active/System/Library/Caches/apticket.der" > apticket.der

        echo "[*] Downloading kernelcache"
        "$dir"/pzb -g AssetData/boot/"$(awk "/""$cpid""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/release/development/')" "$ipswurl"

        echo "[*] Patching kernelcache"
        cd ..
        modelwithoutap=$(echo "$model" | sed 's/ap//')
        bpatchfile=$(find ../patches -name "$modelwithoutap".bpatch)
        "$dir"/img4 -i work/kernelcache.development.* -o work/kernelcache -M work/apticket.der -P "$bpatchfile" `if [ "$os" = 'Linux' ]; then echo "-J"; fi`

        echo "[*] Placing patched kernelcache"
        cat work/kernelcache | "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat > /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd"

        rm -rf work
        mkdir work
    else
        cd work
        echo "[*] Downloading BuildManifest"
        "$dir"/pzb -g BuildManifest.plist "$ipswurl"

        echo "[*] Getting apticket.der from device"
        has_active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "ls /mnt6/active" 2> /dev/null)
        if [ ! "$has_active" = "/mnt6/active" ]; then
            echo "[!] Active file does not exist! Please use SSH to create it"
            echo "    /mnt6/active should contain the name of the UUID in /mnt6"
            echo "    When done, type reboot in the SSH session, then rerun the script"
            echo "    ssh root@localhost -p 2222"
            exit
        fi
        active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /mnt6/active" 2> /dev/null)
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /mnt6/$active/System/Library/Caches/apticket.der" > apticket.der

        echo "[*] Downloading kernelcache"
        "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
        
        echo "[*] Patching kernelcache"
        cd ..
        if [[ "$deviceid" == "iPhone8"* ]] || [[ "$deviceid" == "iPad6"* ]]; then
            python3 -m pyimg4 im4p extract -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o work/kcache.raw --extra work/kpp.bin
        else
            python3 -m pyimg4 im4p extract -i work/"$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" work/BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" -o work/kcache.raw
        fi
        "$dir"/Kernel64Patcher work/kcache.raw work/kcache.patched -a -o
        if [[ "$deviceid" == *'iPhone8'* ]] || [[ "$deviceid" == *'iPad6'* ]] || [[ "$deviceid" == *'iPad5'* ]] && [[ ! $1 == *"--tweaks"* ]]; then
            python3 -m pyimg4 im4p create -i work/kcache.patched -o work/kcache.im4p -f rkrn --extra work/kpp.bin --lzss
        elif [[ ! $1 == *"--tweaks"* ]]; then
            python3 -m pyimg4 im4p create -i work/kcache.patched -o work/kcache.im4p -f rkrn --lzss
        fi
        "$dir"/img4 -i work/kcache.im4p -o work/kernelcache -M work/apticket.der `if [ "$os" = 'Linux' ]; then echo "-J"; fi`

        echo "[*] Placing patched kernelcache"
        cat work/kernelcache | "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat > /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd"

        rm -rf work
        mkdir work
    fi

    sleep 2
    echo "[*] Done! Rebooting your device"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/reboot"
    sleep 1
    _kill_if_running iproxy

    if [ "$1" = "--tweaks" ]; then
        _wait recovery
    else
        sleep 5
        _wait normal
        sleep 2

        echo "[*] Switching device into recovery mode..."
        "$dir"/ideviceenterrecovery $(_info normal UniqueDeviceID)
        _wait recovery
    fi
    sleep 10
    _dfuhelper
    sleep 2
fi

# ============
# Boot create
# ============

# Actually create the boot files
if [ ! -f boot-"$deviceid"/.fsboot ]; then
    rm -rf boot-"$deviceid"
fi

if [ ! -f boot-"$deviceid"/ibot.img4 ]; then
    _pwn

    # if tweaks, set ipswurl to a custom one
    if [ "$1" = "--tweaks" ]; then
        ipswurl=$(_beta_url)
    fi

    # Downloading files, and decrypting iBSS/iBEC
    rm -rf boot-"$deviceid"
    mkdir boot-"$deviceid"

    echo "[*] Converting blob"
    "$dir"/img4tool -e -s $(pwd)/blobs/"$deviceid"-"$version".shsh2 -m work/IM4M
    cd work

    if [[ $1 == *"--tweaks"* ]]; then
        echo "[*] Downloading BuildManifest"
        "$dir"/pzb -g AssetData/boot/BuildManifest.plist "$ipswurl"

        echo "[*] Downloading and decrypting iBSS"
        "$dir"/pzb -g AssetData/boot/"$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/RELEASE/DEVELOPMENT/')" "$ipswurl"
        "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//' | sed 's/RELEASE/DEVELOPMENT/')" iBSS.dec

        echo "[*] Downloading and decrypting iBoot"
        # download iboot and replace RELEASE with DEVELOPMENT
        "$dir"/pzb -g AssetData/boot/"$(awk "/""$cpid""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/RELEASE/DEVELOPMENT/')" "$ipswurl"
        "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//' | sed 's/RELEASE/DEVELOPMENT/')" ibot.dec
    else
        echo "[*] Downloading BuildManifest"
        "$dir"/pzb -g BuildManifest.plist "$ipswurl"

        echo "[*] Downloading and decrypting iBSS"
        "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
        "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBSS.dec

        echo "[*] Downloading and decrypting iBEC"
        "$dir"/pzb -g "$(awk "/""$cpid""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
        "$dir"/gaster decrypt "$(awk "/""$cpid""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" ibot.dec
    fi

    echo "[*] Patching and signing iBSS/iBoot"
    "$dir"/iBoot64Patcher iBSS.dec iBSS.patched
    if [[ $1 == *"--tweaks"* ]]; then
        if [[ "$@" == *"--semi-tethered"* ]]; then
            "$dir"/iBoot64Patcherfsboot ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e rd=disk0s1s8 launchd_unsecure_cache=1 launchd_missing_exec_no_panic=1 amfi=0xff amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 amfi_allow_research=1 amfi_unrestrict_task_for_pid=1 amfi_unrestricted_local_signing=1 cs_enforcement_disable=1 pmap_cs_allow_modified_code_pages=1 pmap_cs_enforce_coretrust=0 pmap_cs_unrestrict_pmap_cs_disable=1 -unsafe_kernel_text dtrace_dof_mode=1 -panic_notify cs_debug=1 PE_i_can_has_debugger=1'
        else
            "$dir"/iBoot64Patcherfsboot ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e launchd_unsecure_cache=1 launchd_missing_exec_no_panic=1 amfi=0xff amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 amfi_allow_research=1 amfi_unrestrict_task_for_pid=1 amfi_unrestricted_local_signing=1 cs_enforcement_disable=1 pmap_cs_allow_modified_code_pages=1 pmap_cs_enforce_coretrust=0 pmap_cs_unrestrict_pmap_cs_disable=1 -unsafe_kernel_text dtrace_dof_mode=1 -panic_notify cs_debug=1 PE_i_can_has_debugger=1'
        fi
    else
        "$dir"/iBoot64Patcherfsboot ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e'
    fi
    if [ "$os" = 'Linux' ]; then
        sed -i 's/\/\kernelcache/\/\kernelcachd/g' ibot.patched
    else
        LC_ALL=C sed -i .bak -e 's/s\/\kernelcache/s\/\kernelcachd/g' ibot.patched
        rm *.bak
    fi
    cd ..
    "$dir"/img4 -i work/iBSS.patched -o boot-"$deviceid"/iBSS.img4 -M work/IM4M -A -T ibss
    "$dir"/img4 -i work/ibot.patched -o boot-"$deviceid"/ibot.img4 -M work/IM4M -A -T `if [[ "$deviceid" == *'iPhone8'* ]] || [[ "$deviceid" == *'iPad6'* ]] || [[ "$deviceid" == *'iPad5'* ]]; then echo "ibec"; else echo "ibss"; fi`

    touch boot-"$deviceid"/.fsboot
fi

# ============
# Boot device
# ============

sleep 2
_pwn
echo "[*] Booting device"
"$dir"/irecovery -f boot-"$deviceid"/iBSS.img4
sleep 1
"$dir"/irecovery -f boot-"$deviceid"/ibot.img4
sleep 1
"$dir"/irecovery -c fsboot

if [ "$os" = 'Darwin' ]; then
    if [ ! "$1" = '--dfu' ]; then
        defaults write -g ignore-devices -bool false
        defaults write com.apple.AMPDevicesAgent dontAutomaticallySyncIPods -bool false
        killall Finder
    fi
fi

cd logs
for file in *.log; do
    mv "$file" SUCCESS_${file}
done
cd ..

rm -rf work rdwork
echo ""
echo "Done!"
echo "The device should now boot to iOS"
echo "If this is your first time jailbreaking, open Tips app and then press Install"
echo "Otherwise, open Tips app and press Do All in the Tools section"
echo "If you have any issues, please join the Discord server and ask for help: https://dsc.gg/palera1n"
echo "Enjoy!"

} | tee logs/"$(date +%T)"-"$(date +%F)"-"$(uname)"-"$(uname -r)".log
