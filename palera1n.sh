#!/usr/bin/env bash

mkdir -p logs
set -e

{

echo "[*] Command ran:`if [ $EUID = 0 ]; then echo " sudo"; fi` ./palera1n.sh $@"

# =========
# Variables
# =========
ipsw="" # IF YOU WERE TOLD TO PUT A CUSTOM IPSW URL, PUT IT HERE. YOU CAN FIND THEM ON https://appledb.dev
version="1.4.0"
os=$(uname)
dir="$(pwd)/binaries/$os"
commit=$(git rev-parse --short HEAD)
branch=$(git rev-parse --abbrev-ref HEAD)
max_args=1
arg_count=0
disk=8
fs=disk0s1s$disk
rd_in_progress=0

# =========
# Functions
# =========
remote_cmd() {
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p2222 root@localhost "$@"
}
remote_cp() {
    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P2222 $@
}

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
    --skip-fakefs       Don't create the fakefs even if --semi-tethered is specified
    --no-baseband       Indicate that the device does not have a baseband
    --restorerootfs     Remove the jailbreak (Actually more than restore rootfs)
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
        --verbose)
            verbose=1
            ;;
        --dfu)
            echo "[!] DFU mode devices are now automatically detected and --dfu is deprecated"
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

recovery_fix_auto_boot() {
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

get_device_mode() {
    if [ "$os" = "Darwin" ]; then
        apples="$(system_profiler SPUSBDataType | grep -B1 'Vendor ID: 0x05ac' | grep 'Product ID:' | cut -dx -f2 | cut -d' ' -f1 | tail -r 2> /dev/null)"
    elif [ "$os" = "Linux" ]; then
        apples="$(lsusb | cut -d' ' -f6 | grep '05ac:' | cut -d: -f2)"
    fi
    local device_count=0
    local usbserials=""
    for apple in $apples; do
        case "$apple" in
            12ab)
            device_mode=normal
            device_count=$((device_count+1))
            ;;
            12a8)
            device_mode=normal
            device_count=$((device_count+1))
            ;;
            1281)
            device_mode=recovery
            device_count=$((device_count+1))
            ;;
            1227)
            device_mode=dfu
            device_count=$((device_count+1))
            ;;
            1222)
            device_mode=diag
            device_count=$((device_count+1))
            ;;
            1338)
            device_mode=checkra1n_stage2
            device_count=$((device_count+1))
            ;;
            4141)
            device_mode=pongo
            device_count=$((device_count+1))
            ;;
        esac
    done
    if [ "$device_count" = "0" ]; then
        device_mode=none
    elif [ "$device_count" -ge "2" ]; then
        echo "[-] Please attach only one device" > /dev/tty
        kill -30 0
        exit 1;
    fi
    if [ "$os" = "Linux" ]; then
        usbserials=$(cat /sys/bus/usb/devices/*/serial)
    elif [ "$os" = "Darwin" ]; then
        usbserials=$(system_profiler SPUSBDataType | grep 'Serial Number' | cut -d: -f2- | sed 's/ //' 2> /dev/null)
    fi
    if grep -qE 'ramdisk tool (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{1,2} [0-9]{1,4} [0-9]{2}:[0-9]{2}:[0-9]{2}' <<< "$usbserials"; then
        device_mode=ramdisk
    fi
    echo "$device_mode"
}

_wait() {
    if [ "$(get_device_mode)" != "$1" ]; then
        echo "[*] Waiting for device in $1 mode"
    fi

    while [ "$(get_device_mode)" != "$1" ]; do
        sleep 1
    done

    if [ "$1" = 'recovery' ]; then
        recovery_fix_auto_boot;
    fi
}

_dfuhelper() {
    local step_one;
    if [[ "$1" = 0x801* && "$deviceid" != *"iPad"* ]]; then
        step_one="Hold volume down + side button"
    else
        step_one="Hold home + power button"
    fi
    echo "[*] Press any key when ready for DFU mode"
    read -n 1 -s
    step 3 "Get ready"
    step 4 "$step_one" &
    sleep 3
    "$dir"/irecovery -c "reset"
    step 1 "Keep holding"
    if [[ "$1" = 0x801* && "$deviceid" != *"iPad"* ]]; then
        step 10 'Release side button, but keep holding volume down'
    else
        step 10 'Release power button, but keep holding home button'
    fi
    sleep 1
    
    if [ "$(get_device_mode)" = "dfu" ]; then
        echo "[*] Device entered DFU!"
    else
        echo "[-] Device did not enter DFU mode, rerun the script and try again"
    fi
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

_exit_handler() {
    [ $? -eq 0 ] && exit
    echo "[-] An error occurred"

    cd logs
    for file in *.log; do
        if [[ "$file" != "SUCCESS_"* ]] || [[ "$file" != "FAIL_"* ]]; then 
            mv "$file" FAIL_${file}
        fi
    done
    cd ..

    echo "[*] A failure log has been made. If you're going ask for help, please attach the latest log."
}
trap _exit_handler EXIT

# ===========
# Fixes
# ===========

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

version=""
parse_cmdline "$@"

if [ "$debug" = "1" ]; then
    set -o xtrace
fi

if [ "$clean" = "1" ]; then
    rm -rf boot* work .tweaksinstalled
    echo "[*] Removed the created boot files"
    exit
fi

if [ -z "$tweaks" ] && [ "$semi_tethered" = "1" ]; then
    echo "[!] --semi-tethered may not be used with rootless"
    echo "    Rootless is already semi-tethered"
    exit 1;
fi

if [ -z "$tweaks" ] && [ "$restorerootfs" = "1" ]; then
    echo "[!] --restorerootfs may not be used with rootless"
    echo "    You can click Remove in Pogo to get rid of the jailbreak"
    exit 1;
fi

if [ "$tweaks" = 1 ] && [ ! -e ".tweaksinstalled" ] && [ ! -e ".disclaimeragree" ] && [ -z "$semi_tethered" ] && [ -z "$restorerootfs" ]; then
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
echo "[*] Waiting for devices"
while [ "$(get_device_mode)" = "none" ]; do
    sleep 1;
done
echo $(echo "[*] Detected $(get_device_mode) mode device" | sed 's/dfu/DFU/')

if grep -E 'pongo|checkra1n_stage2|diag' <<< "$(get_device_mode)"; then
    echo "[-] Detected device in unsupported mode '$(get_device_mode)'"
    exit 1;
fi

if [ "$(get_device_mode)" != "normal" ] && [ -z "$version" ] && [ "$dfuhelper" != "1" ]; then
    echo "[-] You must pass the version your device is on when not starting from normal mode"
    exit
fi

if [ "$(get_device_mode)" = "ramdisk" ]; then
    # If a device is in ramdisk mode, perhaps iproxy is still running?
    _kill_if_running iproxy
    echo "[*] Rebooting device in SSH Ramdisk"
    if [ "$os" = 'Linux' ]; then
        sudo "$dir"/iproxy 2222 22 &
    else
        "$dir"/iproxy 2222 22 &
    fi
    sleep 1
    remote_cmd "/usr/sbin/nvram auto-boot=false"
    remote_cmd "/sbin/reboot"
    _kill_if_running iproxy
    _wait recovery
fi

if [ "$(get_device_mode)" = "normal" ]; then
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

if [ "$dfuhelper" = "1" ]; then
    echo "[*] Running DFU helper"
    _dfuhelper "$cpid"
    exit
fi

if [ ! "$ipsw" = "" ]; then
    ipswurl=$ipsw
else
    #buildid=$(curl -sL https://api.ipsw.me/v4/ipsw/$version | "$dir"/jq '.[0] | .buildid' --raw-output)
    if [[ "$deviceid" == *"iPad"* ]]; then
        device_os=iPadOS
        device=iPad
    elif [[ "$deviceid" == *"iPod"* ]]; then
        device_os=iOS
        device=iPod
    else
        device_os=iOS
        device=iPhone
    fi

    buildid=$(curl -sL https://api.ipsw.me/v4/ipsw/$version | "$dir"/jq '[.[] | select(.identifier | startswith("'$device'")) | .buildid][0]' --raw-output)
    if [ "$buildid" == "19B75" ]; then
        buildid=19B74
    fi
    ipswurl=$(curl -sL https://api.appledb.dev/ios/$device_os\;$buildid.json | "$dir"/jq -r .devices\[\"$deviceid\"\].ipsw)
fi

if [ "$restorerootfs" = "1" ]; then
    rm -rf "blobs/"$deviceid"-"$version".der" "boot-$deviceid" work .tweaksinstalled
fi

# Have the user put the device into DFU
if [ "$(get_device_mode)" != "dfu" ]; then
    recovery_fix_auto_boot;
    _dfuhelper "$cpid"
fi
sleep 2

# ============
# Ramdisk
# ============

# Dump blobs, and install pogo if needed 
if [ ! -f blobs/"$deviceid"-"$version".der ]; then
    mkdir -p blobs

    cd ramdisk
    chmod +x sshrd.sh
    echo "[*] Creating ramdisk"
    ./sshrd.sh `if [[ "$version" == *"16"* ]]; then echo "16.0.3"; else echo "15.6"; fi` `if [ -z "$tweaks" ]; then echo "rootless"; fi`

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

    while ! (remote_cmd "echo connected" &> /dev/null); do
        sleep 1
    done

    rd_in_progress=1
    
    if [ "$tweaks" = "1" ]; then
        echo "[*] Testing for baseband presence"
        if [ "$(remote_cmd "/usr/bin/mgask HasBaseband | grep -E 'true|false'")" = "true" ] && [ "${cpid}" == *"0x7001"* ]; then
            disk=7
        elif [ "$(remote_cmd "/usr/bin/mgask HasBaseband | grep -E 'true|false'")" = "false" ]; then
            if [ "${cpid}" == *"0x7001"* ]; then
                disk=6
            else
                disk=7
            fi
        fi

        if [ -z "$semi_tethered" ]; then
            disk=1
        fi

        if [[ "$version" == *"16"* ]]; then
            fs=disk1s$disk
        else
            fs=disk0s1s$disk
        fi
    fi

    # mount filesystems, no user data partition
    remote_cmd "/usr/bin/mount_filesystems_nouser"

    has_active=$(remote_cmd "ls /mnt6/active" 2> /dev/null)
    if [ ! "$has_active" = "/mnt6/active" ]; then
        echo "[!] Active file does not exist! Please use SSH to create it"
        echo "    /mnt6/active should contain the name of the UUID in /mnt6"
        echo "    When done, type reboot in the SSH session, then rerun the script"
        echo "    ssh root@localhost -p 2222"
        exit
    fi
    active=$(remote_cmd "cat /mnt6/active" 2> /dev/null)

    if [ "$restorerootfs" = "1" ]; then
        echo "[*] Removing Jailbreak"
        remote_cmd "/sbin/apfs_deletefs $fs > /dev/null || true"
        remote_cmd "rm -f /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd"
        remote_cmd "mv /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache.bak /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache || true"
        remote_cmd "/bin/sync"
        remote_cmd "/usr/sbin/nvram auto-boot=true"
        rm -f BuildManifest.plist
        echo "[*] Done! Rebooting your device"
        remote_cmd "/sbin/reboot"
        exit;
    fi

    echo "[*] Dumping apticket"
    sleep 1
    remote_cp root@localhost:/mnt6/$active/System/Library/Caches/apticket.der blobs/"$deviceid"-"$version".der
    #remote_cmd "cat /dev/rdisk1" | dd of=dump.raw bs=256 count=$((0x4000)) 
    #"$dir"/img4tool --convert -s blobs/"$deviceid"-"$version".shsh2 dump.raw
    #rm dump.raw

    if [ "$semi_tethered" = "1" ]; then
        if [ -z "$skip_fakefs" ]; then
            echo "[*] Creating fakefs, this may take a while (up to 10 minutes)"
            remote_cmd "/sbin/newfs_apfs -A -D -o role=r -v System /dev/disk0s1"
            sleep 2
            remote_cmd "/sbin/mount_apfs /dev/$fs /mnt8"
            
            sleep 1
            remote_cmd "cp -a /mnt1/. /mnt8/"
            sleep 1
            echo "[*] fakefs created, continuing..."
        fi
    fi

    #remote_cmd "/usr/sbin/nvram allow-root-hash-mismatch=1"
    #remote_cmd "/usr/sbin/nvram root-live-fs=1"
    if [ "$tweaks" = "1" ]; then
        if [ "$semi_tethered" = "1" ]; then
            remote_cmd "/usr/sbin/nvram auto-boot=true"
        else
            remote_cmd "/usr/sbin/nvram auto-boot=false"
        fi
    else
        remote_cmd "/usr/sbin/nvram auto-boot=true"
    fi

    # lets actually patch the kernel
    echo "[*] Patching the kernel"
    if [[ "$version" == *"16"* ]]; then
        if [ "$semi_tethered" = "1" ]; then
            remote_cp binaries/Kernel16Patcher-nolivefs.ios root@localhost:/mnt1/private/var/root/kpf
        else
            remote_cp binaries/Kernel16Patcher.ios root@localhost:/mnt1/private/var/root/kpf
        fi
    else
        remote_cp binaries/Kernel15Patcher.ios root@localhost:/mnt1/private/var/root/kpf
    fi
    remote_cmd "/usr/sbin/chown 0 /mnt1/private/var/root/kpf"
    remote_cmd "/bin/chmod 755 /mnt1/private/var/root/kpf"

    remote_cmd "rm -f /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd"
    if [ "$tweaks" = "1" ]; then
        if [ "$semi_tethered" = "1" ]; then
            remote_cmd "cp /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache.bak"
        else
            remote_cmd "mv /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcache.bak"
        fi
    fi
    sleep 1

    # download the kernel
    echo "[*] Downloading BuildManifest"
    "$dir"/pzb -g BuildManifest.plist "$ipswurl"

    echo "[*] Downloading kernelcache"
    "$dir"/pzb -g "$(awk "/""$model""/{x=1}x&&/kernelcache.release/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$ipswurl"
    
    echo "[*] Patching kernelcache"
    mv kernelcache.release.* work/kernelcache
    if [[ "$deviceid" == "iPhone8"* ]] || [[ "$deviceid" == "iPad6"* ]] || [[ "$deviceid" == *'iPad5'* ]]; then
        python3 -m pyimg4 im4p extract -i work/kernelcache -o work/kcache.raw --extra work/kpp.bin
    else
        python3 -m pyimg4 im4p extract -i work/kernelcache -o work/kcache.raw
    fi
    sleep 1
    remote_cp work/kcache.raw root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/
    remote_cmd "/mnt1/private/var/root/kpf /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched"
    remote_cp root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched work/
    if [ "$tweaks" = "1" ]; then
        if [[ "$version" == *"16"* ]]; then
            "$dir"/Kernel64Patcher work/kcache.patched work/kcache.patched2 -e -o -u -l
        else
            "$dir"/Kernel64Patcher work/kcache.patched work/kcache.patched2 -e -l
        fi
    fi
    sleep 1
    if [[ "$deviceid" == *'iPhone8'* ]] || [[ "$deviceid" == *'iPad6'* ]] || [[ "$deviceid" == *'iPad5'* ]]; then
        python3 -m pyimg4 im4p create -i work/kcache.patched2 -o work/kcache.im4p -f krnl --extra work/kpp.bin --lzss
    elif [ "$tweaks" = "1" ]; then
        python3 -m pyimg4 im4p create -i work/kcache.patched2 -o work/kcache.im4p -f krnl --lzss
    fi
    sleep 1
    remote_cp work/kcache.im4p root@localhost:/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/
    remote_cmd "img4 -i /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p -o /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd -M /mnt6/$active/System/Library/Caches/apticket.der"
    remote_cmd "rm -f /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.raw /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.patched /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kcache.im4p"

    sleep 1
    has_kernelcachd=$(remote_cmd "ls /mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd" 2> /dev/null)
    if [ "$has_kernelcachd" = "/mnt6/$active/System/Library/Caches/com.apple.kernelcaches/kernelcachd" ]; then
        echo "[*] Custom kernelcache now exists!"
    else
        echo "[!] Custom kernelcache doesn't exist..? Please send a log and report this bug..."
    fi

    if [ "$tweaks" = "1" ]; then
        sleep 1
        remote_cmd "/sbin/mount_apfs /dev/$fs /mnt8 || true"

        # iOS 16 stuff
        if [[ "$version" == *"16"* ]]; then
            if [ -z "$semi_tethered" ]; then
                echo "[*] Performing iOS 16 fixes"
                sleep 1
                os_disk=$(remote_cmd "/usr/sbin/hdik /mnt6/cryptex1/current/os.dmg | head -3 | tail -1 | sed 's/ .*//'")
                sleep 1
                app_disk=$(remote_cmd "/usr/sbin/hdik /mnt6/cryptex1/current/app.dmg | head -3 | tail -1 | sed 's/ .*//'")
                sleep 1
                remote_cmd "/sbin/mount_apfs -o ro $os_disk /mnt2"
                sleep 1
                remote_cmd "/sbin/mount_apfs -o ro $app_disk /mnt9"
                sleep 1

                remote_cmd "rm -rf /mnt8/System/Cryptexes/App /mnt8/System/Cryptexes/OS"
                sleep 1
                remote_cmd "mkdir /mnt8/System/Cryptexes/App /mnt8/System/Cryptexes/OS"
                sleep 1
                remote_cmd "cp -a /mnt9/. /mnt8/System/Cryptexes/App"
                sleep 1
                remote_cmd "cp -a /mnt2/. /mnt8/System/Cryptexes/OS"
                sleep 1
                remote_cmd "rm -rf /mnt8/System/Cryptexes/OS/System/Library/Caches/com.apple.dyld"
                sleep 1
                remote_cmd "cp -a /mnt2/System/Library/Caches/com.apple.dyld /mnt8/System/Library/Caches/"
            fi
        fi

        echo "[*] Copying files to rootfs"
        remote_cmd "rm -rf /mnt8/jbin /mnt8/palera1n"
        sleep 1
        remote_cmd "mkdir -p /mnt8/jbin/binpack /mnt8/jbin/loader.app /mnt8/palera1n"
        sleep 1

        # download loader
        cd other/rootfs/jbin
        rm -rf loader.app
        curl -LO https://nightly.link/palera1n/loader/workflows/build/main/loader.zip
        unzip loader.zip -d .
        unzip palera1n.ipa -d .
        mv Payload/palera1nLoader.app loader.app
        rm -rf loader.zip palera1n.ipa Payload
        cd ../../..

        sleep 1
        remote_cp -r other/rootfs/* root@localhost:/mnt$disk
        {
            echo "{"
            echo "    \"version\": \"${version} (${commit}_${branch})\","
            echo "    \"args\": \"$@\","
            echo "    \"pc\": \"$(uname) $(uname -r)\","

            echo "}"
        } > work/info.json
        sleep 1
        remote_cp work/info.json root@localhost:/mnt$disk/palera1n
        remote_cmd "ldid -s /mnt$disk/jbin/launchd /mnt$disk/jbin/haxx"
        remote_cmd "chmod +x /mnt$disk/jbin/launchd /mnt$disk/jbin/haxx /mnt$disk/jbin/post.sh"
        remote_cmd "tar -xvf /mnt$disk/jbin/binpack/binpack.tar -C /mnt$disk/jbin/binpack/"
        sleep 1
        remote_cmd "rm /mnt$disk/jbin/binpack/binpack.tar"
    fi

    rm -rf work BuildManifest.plist
    mkdir work
    rd_in_progress=0

    sleep 2
    echo "[*] Phase 1 done! Rebooting your device (if it doesn't reboot, you may force reboot)"
    remote_cmd "/sbin/reboot"
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
    _dfuhelper
    sleep 2
fi

# ============
# Boot create
# ============

# Actually create the boot files
if [ ! -f boot-"$deviceid"/.local ]; then
    rm -rf boot-"$deviceid"
fi

if [ ! -f boot-"$deviceid"/ibot.img4 ]; then
    # Downloading files, and decrypting iBSS/iBEC
    rm -rf boot-"$deviceid"
    mkdir boot-"$deviceid"

    #echo "[*] Converting blob"
    #"$dir"/img4tool -e -s $(pwd)/blobs/"$deviceid"-"$version".shsh2 -m work/IM4M
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
        if [ "$verbose" = "1" ]; then
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -b "-v rd=$fs" -l
        else
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -b "rd=$fs" -l
        fi
    else
        if [ "$verbose" = "1" ]; then
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -b '-v' -l
        else
            "$dir"/iBoot64Patcher ibot.dec ibot.patched -l
        fi
    fi

    if [ "$os" = 'Linux' ]; then
        sed -i 's/\/\kernelcache/\/\kernelcachd/g' ibot.patched
    else
        LC_ALL=C sed -i.bak -e 's/s\/\kernelcache/s\/\kernelcachd/g' ibot.patched
        rm *.bak
    fi
    cd ..
    "$dir"/img4 -i work/iBSS.patched -o boot-"$deviceid"/iBSS.img4 -M blobs/"$deviceid"-"$version".der -A -T ibss
    "$dir"/img4 -i work/ibot.patched -o boot-"$deviceid"/ibot.img4 -M blobs/"$deviceid"-"$version".der -A -T `if [[ "$cpid" == *"0x801"* ]]; then echo "ibss"; else echo "ibec"; fi`

    touch boot-"$deviceid"/.local
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
else
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/iBSS.img4
    sleep 4
    "$dir"/irecovery -f boot-"$deviceid"/ibot.img4
fi

cd logs
for file in *.log; do 
    if [[ "$file" != "SUCCESS_"* ]]; then 
        mv "$file" SUCCESS_${file}
    fi
done
cd ..

rm -rf work rdwork
echo ""
echo "Done!"
echo "The device should now boot to iOS"
echo "If this is your first time jailbreaking, open the Pogo app and then press Install"
echo "Otherwise, open the Pogo app and press Do All in the Tools section"
echo "If you have any issues, please join the Discord server and ask for help: https://dsc.gg/palera1n"
echo "Enjoy!"

} | tee logs/"$(date +%T)"-"$(date +%F)"-"$(uname)"-"$(uname -r)".log
