#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

PLATFORM="${1:-iphoneos}"

case "$PLATFORM" in
    iphoneos)
        TARGET_SDK="iphoneos"
        ;;
    *)
        echo "error: unsupported platform: $PLATFORM" >&2
        exit 1
        ;;
esac

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "error: Apple SDK builds require macOS/Xcode" >&2
    exit 1
fi

MACOSX_SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"
TARGET_SYSROOT="$(xcrun --sdk "$TARGET_SDK" --show-sdk-path)"
CUSTOM_INCLUDE_PATH="$ROOT_DIR/apple-include-$PLATFORM"

mkdir -p "$CUSTOM_INCLUDE_PATH"/IOKit/kext
cp -af "$MACOSX_SYSROOT"/System/Library/Frameworks/IOKit.framework/Headers/{AppleConvergedIPCKeys.h,IOBSD.h,IOCFBundle.h,IOCFPlugIn.h,IOCFURLAccess.h,IOKitServer.h,IORPC.h,IOSharedLock.h,IOUserServer.h,audio,avc,firewire,graphics,hid,hidsystem,i2c,iokitmig.h,kext,ndrvsupport,network,ps,pwr_mgt,sbp2,scsi,serial,storage,stream,usb,video} "$CUSTOM_INCLUDE_PATH/IOKit"
cp -af "$MACOSX_SYSROOT"/System/Library/Frameworks/IOKit.framework/Headers/* "$CUSTOM_INCLUDE_PATH/IOKit"
gsed -i -E s/'__API_UNAVAILABLE\(.*\)'// "$CUSTOM_INCLUDE_PATH/IOKit/IOKitLib.h"

