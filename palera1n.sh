#!/usr/bin/env bash

mkdir -p logs
set -e

{

echo "[*] Command ran:`if [ $EUID = 0 ]; then echo " sudo"; fi` ./palera1n.sh $@"

# =========
# Variables
# =========
ipsw="" # IF YOU WERE TOLD TO PUT A CUSTOM IPSW URL, PUT IT HERE. YOU CAN FIND THEM ON https://appledb.dev
version="1.3.0"
os=$(uname)
dir="$(pwd)/binaries/$os"
commit=$(git rev-parse --short HEAD)
branch=$(git rev-parse --abbrev-ref HEAD)
max_args=1
arg_count=0

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

print_help() {
    cat << EOF
Usage: $0 [Options] [ subcommand | iOS version ]
iOS 15.0-15.7.1 jailbreak tool for checkm8 devices

Options:
    --help              Print this help
    --tweaks            Enable tweaks
    --semi-tethered     When used with --tweaks, make the jailbreak semi-tethered instead of tethered
    --dfuhelper         A helper to help get A11 devices into DFU mode from recovery mode
    --no-baseband       When used with --semi-tethered, allows the fakefs to be created correctly on no baseband devices
    --skip-fakefs       Don't create the fakefs even if --semi-tethered is specified
    --no-install        Skip murdering Tips app
    --dfu               Indicate that the device is connected in DFU mode
    --restorerootfs     Restore the root fs on tethered
    --debug             Debug the script
    --verbose           Enable verbose boot on the device

Subcommands:
    dfuhelper           An alias for --dfuhelper
    clean               Deletes the created boot files

The iOS version argument should be the iOS version of your device.
It is required when starting from DFU mode.
EOF
}

parse_opt() {
    case "$1" in
        --)
            no_more_opts=1
            ;;
        --tweaks)
            tweaks=1
            ;;
        --semi-tethered)
            semi_tethered=1
            ;;
        --dfuhelper)
            dfuhelper=1
            ;;
        --skip-fakefs)
            skip_fakefs=1
            ;;
        --no-baseband)
            no_baseband=1
            ;;
        --no-install)
            no_install=1
            ;;
        --verbose)
            verbose=1
            ;;
        --dfu)
            dfu=1
            ;;
        --restorerootfs)
            restorerootfs=1
            ;;
        --debug)
            debug=1
            ;;
        --help)
            print_help
            exit 0
            ;;
        *)
            echo "[-] Unknown option $1. Use $0 --help for help."
            exit 1;
    esac
}

parse_arg() {
    arg_count=$((arg_count + 1))
    case "$1" in
        dfuhelper)
            dfuhelper=1
            ;;
        clean)
            clean=1
            ;;
        *)
            version="$1"
            ;;
    esac
}

parse_cmdline() {
    for arg in $@; do
        if [[ "$arg" == --* ]] && [ -z "$no_more_opts" ]; then
            parse_opt "$arg";
        elif [ "$arg_count" -lt "$max_args" ]; then
            parse_arg "$arg";
        else
            echo "[-] Too many arguments. Use $0 --help for help.";
            exit 1;
        fi
    done
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

        if [ "$tweaks" = "1" ]; then
            "$dir"/irecovery -c "setenv auto-boot false"
            "$dir"/irecovery -c "saveenv"
        else
            "$dir"/irecovery -c "setenv auto-boot true"
            "$dir"/irecovery -c "saveenv"
        fi

        if [ "$semi_tethered" = "1" ]; then
            "$dir"/irecovery -c "setenv auto-boot true"
            "$dir"/irecovery -c "saveenv"
        fi
    fi
}

_check_dfu() {
    if [ "$os" = 'Darwin' ]; then
        if ! (system_profiler SPUSBDataType 2> /dev/null | grep ' Apple Mobile Device (DFU Mode):' >> /dev/null); then
            echo "[-] Connected device is not in DFU mode, please rerun the script and try again"
            exit
        fi
    else
        if ! (lsusb 2> /dev/null | grep 'DFU Mode' >> /dev/null); then
            echo "[-] Connected device is not in DFU mode, please rerun the script and try again"
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
        #"$dir"/gaster reset
        #sleep 1
    fi
}

_reset() {
        echo "[*] Resetting DFU state"
        "$dir"/gaster reset
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
        if [ -z "$dfu" ]; then
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

parse_cmdline "$@"

if [ "$debug" = "1" ]; then
    set -o xtrace
fi

# ===========
# Subcommands
# ===========

if [ "$clean" = "1" ]; then
    rm -rf boot* work .tweaksinstalled
    echo "[*] Removed the created boot files"
    exit
elif [ "$dfuhelper" = "1" ]; then
    echo "[*] Running DFU helper"
    _dfuhelper
    exit
elif [ "$restorerootfs" = "1" ]; then
    echo "[*] Restoring rootfs..."
    "$dir"/irecovery -n
    sleep 2
    echo "[*] Done, your device will boot into iOS now."
    # clean the boot files bcs we don't need them anymore
    rm -rf boot-"$deviceid" work .tweaksinstalled
    exit
fi

if [ -z "$tweaks" ] && [ "$semi_tethered" = "1" ]; then
    echo "[!] --semi-tethered may not be used with rootless"
    echo "    Rootless is already semi-tethered"
    exit 1;
fi

if [ "$tweaks" = "1" ]; then
    _check_dfu
fi

if [ "$tweaks" = 1 ] && [ ! -e ".tweaksinstalled" ] && [ ! -e ".disclaimeragree" ] && [ -z "$semi_tethered" ]; then
    echo "!!! WARNING WARNING WARNING !!!"
    echo "This flag will add tweak support BUT WILL BE TETHERED."
    echo "THIS ALSO MEANS THAT YOU'LL NEED A PC EVERY TIME TO BOOT."
    echo "THIS ONLY WORKS ON 15.0-15.7.1"
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
            exit
        fi
    else
        exit
    fi
fi

# Get device's iOS version from ideviceinfo if in normal mode
if [ "$dfu" = "1" ] || [ "$tweaks" = "1" ]; then
    if [ -z "$version" ]; then
        echo "[-] When using --dfu, please pass the version you're device is on"
        exit
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
    #buildid=$(curl -sL https://api.ipsw.me/v4/ipsw/$version | "$dir"/jq '.[0] | .buildid' --raw-output)
    if [[ "$deviceid" == *"iPad"* ]]; then
        os=iPadOS
        device=iPad
    elif [[ "$deviceid" == *"iPod"* ]]; then
        os=iOS
        device=iPod
    else
        os=iOS
        device=iPhone
    fi

    buildid=$(curl -sL https://api.ipsw.me/v4/ipsw/$version | "$dir"/jq '[.[] | select(.identifier | startswith("'$device'")) | .buildid][0]' --raw-output)
    ipswurl=$(curl -sL https://api.appledb.dev/ios/$os\;$buildid.json | "$dir"/jq -r .devices\[\"$deviceid\"\].ipsw)
fi

# Have the user put the device into DFU
if [ -z "$dfu" ] && [ -z "$tweaks" ]; then
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
    ./sshrd.sh 15.6 `if [ ! "$1" = '--tweaks' ]; then echo "rootless"; fi`

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
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/bin/mount_filesystems"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /dev/rdisk1" | dd of=dump.raw bs=256 count=$((0x4000)) 
    "$dir"/img4tool --convert -s blobs/"$deviceid"-"$version".shsh2 dump.raw
    rm dump.raw

    if [ "$semi_tethered" = "1" ]; then
        if [ -z "$skip_fakefs" ]; then
            echo "[*] Creating fakefs, this may take a while (up to 10 minutes)"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/newfs_apfs -A -D -o role=r -v System /dev/disk0s1"
            sleep 2
            if [ "$skip_baseband" = "1" ]; then 
                "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/mount_apfs /dev/disk0s1s7 /mnt8"
            else
                "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/mount_apfs /dev/disk0s1s8 /mnt8"
            fi
            
            sleep 1
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cp -a /mnt1/. /mnt8/"
            sleep 1
            echo "[*] fakefs created, continuing..."
        fi
    fi

    if [ -z "$no_install" ]; then
        tipsdir=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/bin/find /mnt2/containers/Bundle/Application/ -name 'Tips.app'" 2> /dev/null)
        sleep 1
        if [ "$tipsdir" = "" ]; then
            echo "[!] Tips is not installed. Once your device reboots, install Tips from the App Store and retry"
            "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/reboot"
            sleep 1
            _kill_if_running iproxy
            exit
        fi
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/mkdir -p /mnt1/private/var/root/temp"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/cp -r /usr/local/bin/loader.app/* /mnt1/private/var/root/temp"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/rm -rf /mnt1/private/var/root/temp/Info.plist /mnt1/private/var/root/temp/Base.lproj /mnt1/private/var/root/temp/PkgInfo"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/cp -rf /mnt1/private/var/root/temp/* $tipsdir"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/rm -rf /mnt1/private/var/root/temp"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/chown 33 $tipsdir/Tips"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/chmod 755 $tipsdir/Tips $tipsdir/PogoHelper"
        sleep 1
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/chown 0 $tipsdir/PogoHelper"
    fi

    #"$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram allow-root-hash-mismatch=1"
    #"$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram root-live-fs=1"
    if [ "$semi_tethered" = "1" ]; then
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram auto-boot=true"
    else
        "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/nvram auto-boot=false"
    fi

    has_active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "ls /mnt6/active" 2> /dev/null)
    if [ ! "$has_active" = "/mnt6/active" ]; then
        echo "[!] Active file does not exist! Please use SSH to create it"
        echo "    /mnt6/active should contain the name of the UUID in /mnt6"
        echo "    When done, type reboot in the SSH session, then rerun the script"
        echo "    ssh root@localhost -p 2222"
        exit
    fi
    active=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cat /mnt6/active" 2> /dev/null)

    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P2222 binaries/Kernel15Patcher.ios root@localhost:/mnt1/private/var/root/Kernel15Patcher.ios
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/usr/sbin/chown 0 /mnt1/private/var/root/Kernel15Patcher.ios"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/bin/chmod 755 /mnt1/private/var/root/Kernel15Patcher.ios"

    # lets actually patch the kernel
    echo "[*] Patching the kernel"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "rm -f /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "cp /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache.bak"
    sleep 1
    # download the kernel
    echo "[*] Downloading BuildManifest"
    "$dir"/pzb -g BuildManifest.plist "$ipswurl"
    echo "[*] Downloading kernelcache"
    "$dir"/pzb -g "$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
    mv kernelcache.release.* work/kernelcache
    if [[ "$deviceid" == "iPhone8"* ]] || [[ "$deviceid" == "iPad6"* ]]|| [[ "$deviceid" == *'iPad5'* ]]; then
        python3 -m pyimg4 im4p extract -i work/kernelcache -o work/kcache.raw --extra work/kpp.bin
    else
        python3 -m pyimg4 im4p extract -i work/kernelcache -o work/kcache.raw
    fi
    sleep 1
    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P2222 work/kcache.raw root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/mnt1/private/var/root/Kernel15Patcher.ios /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched"
    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P2222 root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched work/
    "$dir"/Kernel64Patcher work/kcache.patched work/kcache.patched2 -e
    sleep 1
    if [[ "$deviceid" == *'iPhone8'* ]] || [[ "$deviceid" == *'iPad6'* ]] || [[ "$deviceid" == *'iPad5'* ]]; then
        python3 -m pyimg4 im4p create -i work/kcache.patched2 -o work/kcache.im4p -f krnl --extra work/kpp.bin --lzss
    elif [ "$tweaks" = "1" ]; then
        python3 -m pyimg4 im4p create -i work/kcache.patched2 -o work/kcache.im4p -f krnl --lzss
    fi
    sleep 1
    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P2222 work/kcache.im4p root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "img4 -i /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p -o /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd -M /mnt6/$active/System/Library/Caches/apticket.der"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "rm -f /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p"

    sleep 1
    has_kernelcachd=$("$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "ls /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd" 2> /dev/null)
    if [ "$has_kernelcachd" = "/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd" ]; then
        echo "[*] Custom kernelcache now exists!"
    else
        echo "[!] Custom kernelcache doesn't exist..? Please send a log and report this bug..."
    fi

    rm -rf work
    mkdir work

    sleep 2
    echo "[*] Done! Rebooting your device"
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "/sbin/reboot"
    sleep 1
    _kill_if_running iproxy

    if [ "$semi_tethered" = "1" ]; then
        _wait normal
        sleep 5

        echo "[*] Switching device into recovery mode..."
        "$dir"/ideviceenterrecovery $(_info normal UniqueDeviceID)
    elif [ -z "$tweaks" ]; then
        _wait normal
        sleep 5

        echo "[*] Switching device into recovery mode..."
        "$dir"/ideviceenterrecovery $(_info normal UniqueDeviceID)
    fi
    _wait recovery
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
    # Downloading files, and decrypting iBSS/iBEC
    rm -rf boot-"$deviceid"
    mkdir boot-"$deviceid"

    echo "[*] Converting blob"
    "$dir"/img4tool -e -s $(pwd)/blobs/"$deviceid"-"$version".shsh2 -m work/IM4M
    cd work

    echo "[*] Downloading BuildManifest"
    "$dir"/pzb -g BuildManifest.plist "$ipswurl"

    echo "[*] Downloading and decrypting iBSS"
    "$dir"/pzb -g "$(awk "/""$model""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
    "$dir"/gaster decrypt "$(awk "/""$model""/{x=1}x&&/iBSS[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]dfu[/]//')" iBSS.dec

    echo "[*] Downloading and decrypting iBoot"
    "$dir"/pzb -g "$(awk "/""$model""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
    "$dir"/gaster decrypt "$(awk "/""$model""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]all_flash[/]//')" ibot.dec

    echo "[*] Patching and signing iBSS/iBoot"
    "$dir"/iBoot64Patcher iBSS.dec iBSS.patched
    if [ "$semi_tethered" = "1" ]; then
        if [ "$no_baseband" = "1" ]; then 
            if [ "$verbose" = "1" ]; then
                "$dir"/iBoot64Patcher ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e rd=disk0s1s7' -f
            else
                "$dir"/iBoot64Patcher ibot.dec ibot.patched -b 'keepsyms=1 debug=0x2014e rd=disk0s1s7' -f
            fi
        else
            if [ "$verbose" = "1" ]; then
                "$dir"/iBoot64Patcher ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e rd=disk0s1s8' -f
            else
                "$dir"/iBoot64Patcher ibot.dec ibot.patched -b 'keepsyms=1 debug=0x2014e rd=disk0s1s8' -f
            fi
        fi
    else
        if [ "$verbose" = "1" ]; then
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -b '-v keepsyms=1 debug=0x2014e' -f
        else
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -b 'keepsyms=1 debug=0x2014e' -f
        fi
    fi
    if [ "$os" = 'Linux' ]; then
        sed -i 's/\/\kernelcache/\/\kernelcachd/g' ibot.patched
    else
        LC_ALL=C sed -i.bak -e 's/s\/\kernelcache/s\/\kernelcachd/g' ibot.patched
        rm *.bak
    fi
    cd ..
    "$dir"/img4 -i work/iBSS.patched -o boot-"$deviceid"/iBSS.img4 -M work/IM4M -A -T ibss
    "$dir"/img4 -i work/ibot.patched -o boot-"$deviceid"/ibot.img4 -M work/IM4M -A -T `if [[ "$cpid" == *"0x801"* ]]; then echo "ibss"; else echo "ibec"; fi`
    "$dir"/img4 -i other/bootlogo.im4p -o boot-"$deviceid"/bootlogo.img4 -M work/IM4M -A -T rlgo

    touch boot-"$deviceid"/.fsboot
fi

# ============
# Boot device
# ============

sleep 2
_pwn
_reset
echo "[*] Booting device"
if [[ "$cpid" == *"0x801"* ]]; then
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/ibot.img4
    sleep 1
    "$dir"/irecovery -c fsboot
else
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/iBSS.img4
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/ibot.img4
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/bootlogo.img4
    sleep 1
    "$dir"/irecovery -c "setpicture 0x1"
    sleep 1
    "$dir"/irecovery -c fsboot
fi

if [ "$os" = 'Darwin' ]; then
    if [ -z "$dfu" ]; then
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
