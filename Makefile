.PHONY: apple-include payloads palera1n palera1n_xcode palera1n_mingw clean

WITH_GUI ?= 0
WITH_TUI ?= 0
WITH_RAMDISK ?= 1
WITH_BINPACK ?= 1
WITH_STATIC ?= 0
WITH_CIDERRAIN ?= 0
BUILD_TYPE ?= Debug

CMAKE_SYSROOT ?=
CROSS_HOST_TRIPLE ?=
JOBS ?=

PLATFORM ?= host

UNAME_S := $(shell uname -s)

BUILD_ARGS :=
ifneq ($(strip $(JOBS)),)
BUILD_ARGS += --parallel $(JOBS)
endif

ifeq ($(PLATFORM),ios)
	PLATFORM := iphoneos
endif
TARGET_SYSROOT :=
MACOSX_SYSROOT := $(shell xcrun --sdk macosx --show-sdk-path)
CUSTOM_INCLUDE_PATH := apple-include-$(PLATFORM)

ifeq ($(PLATFORM),iphoneos)
ifneq ($(UNAME_S),Darwin)
$(error iOS builds require macOS/Xcode)
endif
TARGET_SYSROOT := $(shell xcrun --sdk iphoneos --show-sdk-path)
else ifeq ($(PLATFORM),macos)
ifneq ($(UNAME_S),Darwin)
$(error macOS SDK builds require macOS/Xcode)
endif
TARGET_SYSROOT := $(shell xcrun --sdk macosx --show-sdk-path)
endif

CMAKE_COMMON_ARGS := \
	-DCMAKE_SYSROOT="$(CMAKE_SYSROOT)" \
	-DCROSS_HOST_TRIPLE="$(CROSS_HOST_TRIPLE)" \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DWITH_GUI=$(WITH_GUI) \
	-DWITH_TUI=$(WITH_TUI) \
	-DWITH_RAMDISK=$(WITH_RAMDISK) \
	-DWITH_BINPACK=$(WITH_BINPACK) \
	-DWITH_CIDERRAIN=$(WITH_CIDERRAIN) \
	-DWITH_STATIC=$(WITH_STATIC)

CMAKE_APPLE_ARGS :=

ifeq ($(PLATFORM),iphoneos)

CMAKE_APPLE_ARGS := \
	-DCMAKE_SYSTEM_NAME=iOS \
	-DCMAKE_OSX_SYSROOT="$(TARGET_SYSROOT)" \
	-DCMAKE_OSX_ARCHITECTURES=arm64 \
	-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0

endif

ifeq ($(PLATFORM),macos)

CMAKE_APPLE_ARGS := \
	-DCMAKE_SYSTEM_NAME=Darwin \
	-DCMAKE_OSX_SYSROOT="$(TARGET_SYSROOT)"

endif

ifeq ($(PLATFORM),iphoneos)
APPLE_INCLUDE_DEP := apple-include
else
APPLE_INCLUDE_DEP :=
endif

apple-include:
	mkdir -p $(CUSTOM_INCLUDE_PATH)/{bsm,objc,os/internal,sys,firehose,CoreFoundation,FSEvents,IOSurface,IOKit/kext,libkern,kern,arm,{mach/,}machine,CommonCrypto,Security,CoreSymbolication,Kernel/{kern,IOKit,libkern},rpc,rpcsvc,xpc/private,ktrace,mach-o,dispatch}
	cp -af $(MACOSX_SYSROOT)/usr/include/{arpa,bsm,hfs,net,xpc,netinet,servers,timeconv.h,launch.h} $(CUSTOM_INCLUDE_PATH)
	cp -af $(MACOSX_SYSROOT)/usr/include/objc/objc-runtime.h $(CUSTOM_INCLUDE_PATH)/objc
	cp -af $(MACOSX_SYSROOT)/usr/include/libkern/{OSDebug.h,OSKextLib.h,OSReturn.h,OSThermalNotification.h,OSTypes.h,machine} $(CUSTOM_INCLUDE_PATH)/libkern
	cp -af $(MACOSX_SYSROOT)/usr/include/kern $(CUSTOM_INCLUDE_PATH)
	cp -af $(MACOSX_SYSROOT)/usr/include/sys/{tty*,ptrace,kern*,random,reboot,user,vnode,disk,vmmeter,conf}.h $(CUSTOM_INCLUDE_PATH)/sys
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/Kernel.framework/Versions/Current/Headers/sys/disklabel.h $(CUSTOM_INCLUDE_PATH)/sys
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/IOKit.framework/Headers/{AppleConvergedIPCKeys.h,IOBSD.h,IOCFBundle.h,IOCFPlugIn.h,IOCFURLAccess.h,IOKitServer.h,IORPC.h,IOSharedLock.h,IOUserServer.h,audio,avc,firewire,graphics,hid,hidsystem,i2c,iokitmig.h,kext,ndrvsupport,network,ps,pwr_mgt,sbp2,scsi,serial,storage,stream,usb,video} $(CUSTOM_INCLUDE_PATH)/IOKit
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/Security.framework/Headers/{mds_schema,oidsalg,SecKeychainSearch,certextensions,Authorization,eisl,SecDigestTransform,SecKeychainItem,oidscrl,cssmcspi,CSCommon,cssmaci,SecCode,CMSDecoder,oidscert,SecRequirement,AuthSession,SecReadTransform,oids,cssmconfig,cssmkrapi,SecPolicySearch,SecAccess,cssmtpi,SecACL,SecEncryptTransform,cssmapi,cssmcli,mds,x509defs,oidsbase,SecSignVerifyTransform,cssmspi,cssmkrspi,SecTask,cssmdli,SecAsn1Coder,cssm,SecTrustedApplication,SecCodeHost,SecCustomTransform,oidsattr,SecIdentitySearch,cssmtype,SecAsn1Types,emmtype,SecTransform,SecTrustSettings,SecStaticCode,emmspi,SecTransformReadTransform,SecKeychain,SecDecodeTransform,CodeSigning,AuthorizationPlugin,cssmerr,AuthorizationTags,CMSEncoder,SecEncodeTransform,SecureDownload,SecAsn1Templates,AuthorizationDB,SecCertificateOIDs,cssmapple}.h $(CUSTOM_INCLUDE_PATH)/Security
# 	cp -af $(MACOSX_SYSROOT)/usr/include/{ar,bootstrap,launch,libc,libcharset,localcharset,nlist,NSSystemDirectories,tzfile,vproc}.h $(CUSTOM_INCLUDE_PATH)
	cp -af $(MACOSX_SYSROOT)/usr/include/mach/{*.defs,{mach_vm,shared_region}.h} $(CUSTOM_INCLUDE_PATH)/mach
	cp -af $(MACOSX_SYSROOT)/usr/include/mach/machine/*.defs $(CUSTOM_INCLUDE_PATH)/mach/machine
	cp -af $(MACOSX_SYSROOT)/usr/include/rpc/pmap_clnt.h $(CUSTOM_INCLUDE_PATH)/rpc
	cp -af $(MACOSX_SYSROOT)/usr/include/rpcsvc/yp{_prot,clnt}.h $(CUSTOM_INCLUDE_PATH)/rpcsvc
	cp -af $(TARGET_SYSROOT)/usr/include/mach/machine/thread_state.h $(CUSTOM_INCLUDE_PATH)/mach/machine
	cp -af $(TARGET_SYSROOT)/usr/include/mach/arm $(CUSTOM_INCLUDE_PATH)/mach
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/IOKit.framework/Headers/* $(CUSTOM_INCLUDE_PATH)/IOKit
# 	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/stdlib.h > $(CUSTOM_INCLUDE_PATH)/stdlib.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/time.h > $(CUSTOM_INCLUDE_PATH)/time.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/unistd.h > $(CUSTOM_INCLUDE_PATH)/unistd.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/task.h > $(CUSTOM_INCLUDE_PATH)/mach/task.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/mach_host.h > $(CUSTOM_INCLUDE_PATH)/mach/mach_host.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/ucontext.h > $(CUSTOM_INCLUDE_PATH)/ucontext.h
#	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/signal.h > $(CUSTOM_INCLUDE_PATH)/signal.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/spawn.h > $(CUSTOM_INCLUDE_PATH)/spawn.h
	gsed -E /'__API_UNAVAILABLE'/d < $(TARGET_SYSROOT)/usr/include/pthread.h > $(CUSTOM_INCLUDE_PATH)/pthread.h
	gsed -i -E s/'__API_UNAVAILABLE\(.*\)'// $(CUSTOM_INCLUDE_PATH)/IOKit/IOKitLib.h
	gsed -i -E s/'__API_UNAVAILABLE\(.*\)'// $(CUSTOM_INCLUDE_PATH)/spawn.h
	gsed -i -E s/'API_UNAVAILABLE\(.*\)'// $(CUSTOM_INCLUDE_PATH)/xpc/*.h
	gsed -i 's|// __XPC_INDIRECT__|\n#include "$(TARGET_SYSROOT)/usr/include/bsm/audit.h"\n|' $(CUSTOM_INCLUDE_PATH)/xpc/connection.h

payloads:
	mkdir -p src/gen/embedded

	xz --format=lzma -vfc6ekT 0 embedded/ramdisk.dmg > embedded/ramdisk-compressed.dmg.lzma
	xz --format=lzma -vfc6ekT 0 embedded/checkra1n-kpf-pongo > embedded/checkra1n-kpf-pongo-compressed.lzma

	@for file in embedded/*; do \
		name=$$(basename "$$file"); \
		name=$${name%.*}; \
		echo " XXD    $$file"; \
		echo "#pragma once" > "src/gen/embedded/$$name.h"; \
		xxd -i "$$file" \
		| sed 's/unsigned char/static const unsigned char/g' \
		| sed 's/unsigned int/static const size_t/g' \
		>> "src/gen/embedded/$$name.h"; \
	done

palera1n: payloads
	@cmake -S . -B build \
		$(CMAKE_COMMON_ARGS) \
		$(CMAKE_APPLE_ARGS) && \
	cmake --build build $(BUILD_ARGS)

palera1n_xcode: $(APPLE_INCLUDE_DEP) payloads
	@cmake -S . -B build \
		-G Xcode \
		$(CMAKE_COMMON_ARGS) \
		$(CMAKE_APPLE_ARGS)

palera1n_mingw: payloads
	@cmake -S . -B build \
		-G "MinGW Makefiles" \
		$(CMAKE_COMMON_ARGS) && \
	cmake --build build $(BUILD_ARGS)

clean:
	@rm -rf build src/gen
	@rm -rf apple-include-*
