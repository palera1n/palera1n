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
|                           | iPad Air 2		|								|								|


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
> AMD CPUs (not AMD Mobile) have an issue where it causes them to have a very low success rate with checkm8 exploit. It is not recommended that you use them with palera1n.

> USB-C port on Apple Silicon Macs *may* require manual unplugging and replugging of the lightning cable after checkm8 exploit. This problem may be solved by connecting via USB hub, though extensions can vary.

## Installing
Visit https://palera.in

## Building

Building is going to be a bit convoluted for each platform, each having their own unique specifications, but the best reference for building should be looking at how [GitHub actions](./.github/workflows/build.yml) does it.

### macOS

Install necessary tooling:

- [Xcode](https://developer.apple.com/xcode/) and [Command Line Tools](https://developer.apple.com/download/all/).
```sh
brew install cmake \
    autoconf \
    automake \
    libtool \
    pkg-config
```

Then compile:

```sh
make palera1n WITH_STATIC=1 WITH_GUI=1
```

### Debian / Ubuntu Linux

Install necessary tooling:

```sh
sudo apt install -y build-essential \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config
```

Then compile:

```sh
make palera1n WITH_STATIC=1 WITH_GUI=1
```

### Windows

- [MSYS2](https://www.msys2.org/)

Open mingw64 terminal and install necessary tooling:

```sh
pacman -S git \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
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
make palera1n_mingw WITH_STATIC=1 WITH_GUI=1
```

## Acknowledgements

- [mineek](https://github.com/mineek) - Checkm8 exploit re-implementation (openra1n) using [gaster](https://github.com/0x7ff/gaster).
- [alfiecg24](https://github.com/mineek) - Achilles, based on openra1n.
- [kok3shidoll](https://github.com/kok3shidoll) - PongoOS patches.
- [itsnebulalol](https://github.com/itsnebulalol) - Palera1n v1.
- [staturnz](https://github.com/staturnz) - Contributions, Loader.
- [plooshi](https://github.com/plooshi) - Plooshfinder, various legacy patches.
- [nekohaxx](https://github.com/nekohaxx) - TUI.
- [checkra1n](https://checkra.in) - Checkra1n (1337), PongoOS, payloads, DFU helper & device assets.
- [libimobiledevice](https://github.com/libimobiledevice/libimobiledevice) - Libimobiledevice, libirecovery, libplist & usbmuxd.
- [libusb](https://github.com/libimobiledevice/libimobiledevice) - Library for access to usb-devices, used by libimobiledevice & openra1n.
- [Procursus](https://github.com/ProcursusTeam/Procursus) - Bootstrap & binpack.
- [tealbathingsuit](https://github.com/tealbathingsuit) - ElleKit.
- [sbingner](https://github.com/sbingner) - Substitute.
- [elihwyma](https://github.com/elihwyma) - Sileo & Pogo.
- [lrdsnow](https://github.com/lrdsnow) - tvOS package manager (PurePKG).
- [tihmstar](https://github.com/tihmstar) - jbinit v1
- [opa334](https://github.com/opa334) - cfprefs hook

## License

Project is licensed under the MIT license. You can see the full details of the license [here](https://github.com/khcrysalis/PlumeImpactor/blob/main/LICENSE).
