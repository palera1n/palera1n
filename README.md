# palera1n

Jailbreak for A8 through A11, T2 devices, on iOS/iPadOS/tvOS 15.0, bridgeOS 5.0 and higher.

## Device Support

<!--- Mobile --->

| iPhone(s)                 | iPad(s)                        		| iPod(s)   					| Apple TV(s) 					|
|-							|-										|-								|-								|
| iPhone 6s                 | iPad mini 4							| iPod Touch (7th generation)	| Apple TV HD                 	|
| iPhone 6s Plus            | iPad (5th generation)					|								| Apple TV 4K (1st generation)	|
| iPhone SE (2016)          | iPad (6th generation)					|								|								|
| iPhone 7                  | iPad (7th generation)					|								|								|
| iPhone 7 Plus             | iPad Pro (9.7")						|								|								|
| iPhone 8                  | iPad Pro (12.9") (1st generation)		|								|								|
| iPhone 8 Plus             | iPad Pro (10.5")						|								|								|
| iPhone X                  | iPad Pro (12.9") (2nd generation)		|								|								|
|                           | iPad Air 2                            |								|								|


> Note that on `A11` (iPhone X, 8, 8 Plus), **you must disable your passcode while in the jailbroken state** (on iOS 16, you need to **reset your device** before proceeding with palera1n).

<!--- T2 --->
<details>
<summary>Apple T2 Device Support (click to expand)</summary>

| Apple T2              	|
|-							|
| Apple T2 iMac20,1         |
| Apple T2 iMac20,2         |
| 			              	|
| Apple T2 MacBookAir8,1    |
| Apple T2 MacBookAir8,2    |
| Apple T2 MacBookAir9,1    |
| 			              	|
| Apple T2 MacBookPro15,1   |
| Apple T2 MacBookPro15,2   |
| Apple T2 MacBookPro15,3   |
| Apple T2 MacBookPro15,4   |
| Apple T2 MacBookPro16,1   |
| Apple T2 MacBookPro16,2   |
| Apple T2 MacBookPro16,3   |
| Apple T2 MacBookPro16,4   |
| 			              	|
| Apple T2 iMacPro1,1       |
| Apple T2 Macmini8,1       |
| Apple T2 MacPro7,1        |
|			              	|
| iBridge2,11 (Unknown Mac) |
| iBridge2,13 (Unknown Mac) |



</details>

## Computer Requirements

1. **USB-A** cables are recommended to use, USB-C to may have issues with palera1n and getting into DFU mode.
> Due to USB-C cables having different accessory IDs, your device may not be able to be recognized when using USB-C due to not being able to assert to its USB voltage pin.

2. **Linux or macOS computer**

> USB-C port on Apple Silicon Macs *may* require manual unplugging and replugging of the lightning cable after checkm8 exploit. This problem may be solved by connecting via USB hub, though extensions can vary.

## Installing

Visit https://palera.in

## Building

Building is going to be a bit convoluted for each platform, each having their own unique specifications, but the best reference for building should be looking at how [GitHub actions](./.github/workflows/build.yml) does it.

```sh
# Supported params:
#  GUI support                              (0 default):
#   WITH_GUI=1
#  TUI support                              (0 default):
#   WITH_TUI=1
#  Default ramdisk                          (1 default):
#   WITH_RAMDISK=1
#  Default overlay                          (1 default):
#   WITH_BINPACK=1
#  Compile statically                       (0 default):
#   WITH_STATIC=0
#  Compile using libciderra1n as backend    (0 default):
#   WITH_CIDERRAIN=0
#  Debug or release build                   ("Debug" default):
#   BUILD_TYPE="Release"
```

### macOS

Install necessary tooling:

- [Xcode](https://developer.apple.com/xcode/) and [Command Line Tools](https://developer.apple.com/download/all/)
- [Rust](https://rustup.rs/)
```sh
brew install cmake \
    autoconf \
    automake \
    libtool \
    pkg-config
```

Then compile:

```sh
make palera1n"
```

### Debian / Ubuntu Linux

Install necessary tooling:

- [Rust](https://rustup.rs/)

```sh
sudo apt install -y \
    build-essential \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config
```

Then compile:

```sh
make palera1n
```

### Windows

- [MSYS2](https://www.msys2.org/)

Open mingw64 terminal and install necessary tooling:

```sh
pacman -S \
    git \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-libusb \
    mingw-w64-x86_64-rust \
    libtool \
    openssl \
    openssl-devel \
    python-devel \
    autoconf \
    automake \
    zip \
    vim \
    make \
    patch
```

Then compile:

```sh
make palera1n_mingw
```

## Acknowledgements

- [mineek](https://github.com/mineek) - Checkm8 exploit re-implementation (libopenra1n), based off [gaster](https://github.com/0x7ff/gaster)
- [kok3shidoll](https://github.com/kok3shidoll) - Checkm8 (1337) exploit re-implementation (libciderra1n), PongoOS patches, ROP chain generator
- [checkra1n](https://checkra.in) - Checkra1n, PongoOS, payloads, DFU helper & device assets
- [staturnz](https://github.com/staturnz) - Liteusb, Loader contributions
- [libusb](https://github.com/libimobiledevice/libimobiledevice) - Library for access to usb-devices, used by libopenra1n
- [idevice](https://github.com/jkcoxson/idevice) - Library for access to usbmuxd devices
- [plooshi](https://github.com/plooshi) - Plooshfinder, plooshinit, various legacy patches
- [Procursus](https://github.com/ProcursusTeam/Procursus) - Bootstrap & binpack
- [tealbathingsuit](https://github.com/tealbathingsuit) - Tweak injection ([ElleKit](https://github.com/tealbathingsuit/ellekit))
- [sbingner](https://github.com/sbingner) - Substitute
- [elihwyma](https://github.com/elihwyma) - Package manager ([Sileo](https://github.com/Sileo/Sileo)) & [Pogo](https://github.com/elihwyma/Pogo)
- [kirb](https://github.com/kirb) - Package manager ([Zebra](https://github.com/zbrateam/Zebra))
- [lrdsnow](https://github.com/lrdsnow) - TvOS package manager ([PurePKG](https://github.com/Lrdsnow/purepkg)).
- [opa334](https://github.com/opa334) - Cfprefs hook
- [itsnebulalol](https://github.com/itsnebulalol) - Palera1n v1
- [llsc12](https://github.com/llsc12) - Palera1n loader v1
- [tihmstar](https://github.com/tihmstar) - Jbinit v1
- [nekohaxx](https://github.com/nekohaxx) - Contributions

## License

Project is licensed under the MIT license. You can see the full details of the license [here](https://github.com/khcrysalis/PlumeImpactor/blob/main/LICENSE). Some components may be licensed under different licenses, see their respective directories for details.

Many things included in this program are made and developed by the Checkra1n team. Please do NOT contact the Checkra1n team for Palera1n, they are not responsible for this program. The original Checkra1n payloads are from [here](https://checkra.in/1337).
