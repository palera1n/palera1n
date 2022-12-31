#!/usr/bin/env bash

mkdir -p logs
set -e 

log="$(date +%T)"-"$(date +%F)"-"$(uname)"-"$(uname -r)".log
cd logs
touch "$log"
cd ..

{

echo "[*] Command ran:`if [ $EUID = 0 ]; then echo " sudo"; fi` ./palera1n.sh $@"

# =========
# Variables
# =========
ipsw="" # IF YOU WERE TOLD TO PUT A CUSTOM IPSW URL, PUT IT HERE. YOU CAN FIND THEM ON https://appledb.dev
version="1.4.1"
os=$(uname)
dir="$(pwd)/binaries/$os"
commit=$(git rev-parse --short HEAD)
branch=$(git rev-parse --abbrev-ref HEAD)
max_args=1
arg_count=0
disk=1
fs=disk0s1s$disk

# =========
# Functions
# =========
remote_cmd() {
    "$dir"/sshpass -p 'alpine' ssh -o StrictHostKeyChecking=no -p6413 root@localhost "$@"
}

remote_cp() {
    "$dir"/sshpass -p 'alpine' scp -o StrictHostKeyChecking=no -P6413 $@
}

step() {
    for i in $(seq "$1" -1 0); do
        if [ "$(get_device_mode)" = "dfu" ]; then
            break
        fi
        printf '\r\e[K\e[1;36m%s (%d)' "$2" "$i"
        sleep 1
    done
    printf '\e[0m\n'
}

print_help() {
    cat << EOF
Usage: $0 [Options] [ subcommand | iOS version ]
iOS 15.0-16.2 jailbreak tool for checkm8 devices

Options:
    --help              Print this help
    --dfuhelper         A helper to help get A11 devices into DFU mode from recovery mode
    --no-baseband       Indicate that the device does not have a baseband
    --debug             Debug the script
    --serial            Enable serial output on the device (only needed for testing with a serial cable)

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
        --dfuhelper)
            dfuhelper=1
            ;;
        --no-baseband)
            no_baseband=1
            ;;
        --serial)
            serial=1
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
    "$dir"/irecovery -c "setenv auto-boot true"
    "$dir"/irecovery -c "saveenv"
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
        apples="$(system_profiler SPUSBDataType 2> /dev/null | grep -B1 'Vendor ID: 0x05ac' | grep 'Product ID:' | cut -dx -f2 | cut -d' ' -f1 | tail -r)"
    elif [ "$os" = "Linux" ]; then
        apples="$(lsusb | cut -d' ' -f6 | grep '05ac:' | cut -d: -f2)"
    fi
    local device_count=0
    local usbserials=""
    for apple in $apples; do
        case "$apple" in
            12a8|12aa|12ab)
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
        usbserials=$(system_profiler SPUSBDataType 2> /dev/null | grep 'Serial Number' | cut -d: -f2- | sed 's/ //')
    fi
    if grep -qE '(ramdisk tool|SSHRD_Script) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{1,2} [0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}' <<< "$usbserials"; then
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
    deviceid=$( [ -z "$deviceid" ] && _info normal ProductType || echo $deviceid )
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
    "$dir"/irecovery -c "reset" &
    wait
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
        return -1
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

    if [ -d "logs" ]; then
        cd logs
        mv "$log" FAIL_${log}
        cd ..
    fi

    echo "[*] A failure log has been made. If you're going ask for help, please attach the latest log."
}
trap _exit_handler EXIT

# ===========
# Fixes
# ===========

# ============
# Dependencies
# ============

# Check for required commands
if [ "$os" = 'Linux' ]; then
    linux_cmds='lsusb'
fi

for cmd in curl unzip python3 git ssh scp killall sudo grep pgrep ${linux_cmds}; do
    if ! command -v "${cmd}" > /dev/null; then
        echo "[-] Command '${cmd}' not installed, please install it!";
        cmd_not_found=1
    fi
done
if [ "$cmd_not_found" = "1" ]; then
    exit 1
fi

# Download gaster
if [ -e "$dir"/gaster ]; then
    "$dir"/gaster &> /dev/null > /dev/null | grep -q 'usb_timeout: 5' && rm "$dir"/gaster
fi

if [ ! -e "$dir"/gaster ]; then
    curl -sLO https://static.palera.in/deps/gaster-"$os".zip
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
echo "Written by Nebula and Mineek | Help from Nathan, Nick Chan and Sunsetplooshi"
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
        sudo "$dir"/iproxy 6413 22 &
    else
        "$dir"/iproxy 6413 22 &
    fi
    sleep 2
    remote_cmd "/usr/sbin/nvram auto-boot=false"
    remote_cmd "/sbin/reboot"
    _kill_if_running iproxy
    _wait recovery
fi

if [ "$(get_device_mode)" = "normal" ]; then
    version=${version:-$(_info normal ProductVersion)}
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
    _dfuhelper "$cpid" || {
        echo "[-] failed to enter DFU mode, run palera1n.sh again"
        exit -1
    }
fi
sleep 2

# ============
# Checks
# ============
if [[ "$deviceid" != iPhone9,[1-4] ]] && [[ "$deviceid" != "iPhone10,"* ]]; then
    echo "[-] This device is unsupported on rootless for now."
    exit
fi

# ============
# Boot create
# ============

# Actually create the boot files
disk=1
if [[ "$version" == *"16"* ]]; then
    fs=disk1s$disk
else
    fs=disk0s1s$disk
fi

boot_args="rootdev=md0"
if [ "$serial" = "1" ]; then
    boot_args="serial=3 rootdev=md0"
else
    boot_args="-v rootdev=md0"
fi

if [[ "$deviceid" == iPhone9,[1-4] ]] || [[ "$deviceid" == "iPhone10,"* ]]; then
    if [ ! -f boot-"$deviceid"/.payload ]; then
        rm -rf boot-"$deviceid"
    fi
else
    if [ ! -f boot-"$deviceid"/.local ]; then
        rm -rf boot-"$deviceid"
    fi
fi

if [ ! -f boot-"$deviceid"/ibot.img4 ]; then
    # Downloading files, and decrypting iBSS/iBEC
    rm -rf boot-"$deviceid"
    mkdir boot-"$deviceid"

    echo "[*] blob moment"
    "$dir"/img4tool -e -s $(pwd)/shsh/"$cpid".shsh2 -m blobs/"$deviceid"-"$version".der
    cd work

    # Do payload if on iPhone 7-X
    if [[ "$deviceid" == iPhone9,[1-4] ]] || [[ "$deviceid" == "iPhone10,"* ]]; then
        if [[ "$version" == "16.0"* ]] || [[ "$version" == "15"* ]]; then
            newipswurl="$ipswurl"
        else
            buildid=$(curl -sL https://api.ipsw.me/v4/ipsw/16.0.3 | "$dir"/jq '[.[] | select(.identifier | startswith("'iPhone'")) | .buildid][0]' --raw-output)
            newipswurl=$(curl -sL https://api.appledb.dev/ios/iOS\;$buildid.json | "$dir"/jq -r .devices\[\"$deviceid\"\].ipsw)
        fi

        echo "[*] Downloading BuildManifest"
        "$dir"/pzb -g BuildManifest.plist "$newipswurl"

        echo "[*] Downloading and decrypting iBoot"
        "$dir"/pzb -g "$(awk "/""$model""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1)" "$newipswurl"
        "$dir"/gaster decrypt "$(awk "/""$model""/{x=1}x&&/iBoot[.]/{print;exit}" BuildManifest.plist | grep '<string>' | cut -d\> -f2 | cut -d\< -f1 | sed 's/Firmware[/]all_flash[/]//')" ibot.dec

        echo "[*] Patching and signing iBoot"
        "$dir"/iBoot64Patcher ibot.dec ibot.patched

        if [[ "$deviceid" == iPhone9,[1-4] ]]; then
            "$dir"/iBootpatch2 --t8010 ibot.patched ibot.patched2
        else
            "$dir"/iBootpatch2 --t8015 ibot.patched ibot.patched2
        fi

        if [ "$os" = 'Linux' ]; then
            sed -i 's/\/\kernelcachd/\/\kernelcache/g' ibot.patched2
        else
            LC_ALL=C sed -i.bak -e 's/s\/\kernelcachd/s\/\kernelcache/g' ibot.patched2
            rm *.bak
        fi

        cd ..
        "$dir"/img4 -i work/ibot.patched2 -o boot-"$deviceid"/ibot.img4 -M blobs/"$deviceid"-"$version".der -A -T ibss

        touch boot-"$deviceid"/.payload
    else
        echo "Sorry, this device is not supported yet on rootless."
        exit 1
    fi
    if [[ "$version" == "15"* ]]; then
        "$dir"/img4 -i other/rootless/rd15.dmg -o boot-"$deviceid"/rd.img4 -M blobs/"$deviceid"-"$version".der -A -T rdsk
    elif [[ "$version" == "16"* ]]; then
        "$dir"/img4 -i other/rootless/rd16.dmg -o boot-"$deviceid"/rd.img4 -M blobs/"$deviceid"-"$version".der -A -T rdsk
    fi
fi

# ============
# Boot device
# ============

sleep 2
_pwn
_reset
echo "[*] Booting device"
if [[ "$deviceid" == iPhone9,[1-4] ]] || [[ "$deviceid" == "iPhone10,"* ]]; then
    sleep 1
    "$dir"/irecovery -f boot-"$deviceid"/ibot.img4
    sleep 3
    "$dir"/irecovery -f boot-"$deviceid"/rd.img4
    sleep 1
    "$dir"/irecovery -c "ramdisk"
    sleep 1
    "$dir"/irecovery -c "dorwx"
    sleep 2
    if [[ "$deviceid" == iPhone9,[1-4] ]]; then
        "$dir"/irecovery -f other/payload/payload_t8010.bin
    else
        "$dir"/irecovery -f other/payload/payload_t8015.bin
    fi
    sleep 3
    "$dir"/irecovery -c "go"
    sleep 1
    "$dir"/irecovery -c "go xargs $boot_args"
    sleep 1
    "$dir"/irecovery -c "go xfb"
    sleep 1
    "$dir"/irecovery -c "go boot md0"
else
    echo "Sorry, this device is not supported yet on rootless."
    exit 1
fi

if [ -d "logs" ]; then
    cd logs
     mv "$log" SUCCESS_${log}
    cd ..
fi

rm -rf work rdwork
echo ""
echo "Done!"
echo "The device should now boot to iOS"
echo "Rootless is WIP, not all tweaks work, and some may even cause bootloops"
echo "=============================================================="
echo "To bootstrap please use ./bootstrap.sh in the rootless folder"
echo "=============================================================="
echo "Enjoy!"
echo " <3 from the palera1n team"

} 2>&1 | tee logs/${log}
