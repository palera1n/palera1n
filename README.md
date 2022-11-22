<h1 align="center">
    <img src="https://avatars.githubusercontent.com/u/114239186?s=100&v=4" alt="palera1n logo">
    <p>palera1n</p>
</h1>
<h3 align="center">An iOS 15.0-15.7.1 work-in-progress, (semi-)tethered checkm8 jailbreak.</h3>
<p align="center">
    <strong><a href="CHANGELOG.md">Change Log</a></strong>
    •
    <strong><a href="https://dsc.gg/palera1n">Discord</a></strong>
    •
    <strong><a href="https://twitter.com/palera1n">Twitter</a></strong>
</p>

# How does it work?
It boots the device with multiple patches required. On first run, it'll boot a ramdisk which dumps your onboard blob, creates a fakefs (if using semi tethered), installs the loader app, and patches your kernel.

# Issues
### Need help?
If you need help, **please** join our Discord. We disabled issues due to the flood of spam, and difficulty to respond in general. We are much more comfortable on Discord.

Please, please, please, provide necessary info:

- iOS version and device (eg. iPhone 7+ 15.1, iPhone 6s 15.3.1)
- Computer's OS and version (eg. Ubuntu 22.04, macOS 13.0)
- The command you ran
- **Full log from the logs folder**

**DO NOT** harass tweak devs if tweaks don't work. Refer to [here](https://github.com/itsnebulalol/ios15-tweaks) for compatiblity.

You may join [here](https://dsc.gg/palera1n).

### Linux
- Linux has some weird usbmuxd issues. We have tried our best to fix them, but there still are issues. We highly recommend to compile and install usbmuxd2.
- Stop making issues about Linux not being able to connect, we are aware. This includes being stuck on waiting for ramdisk to finish booting.

### Warning
- We are **NOT** responsible for any data loss. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. **If your device is stuck in recovery, please run one of the following:**
   - futurerestore --exit-recovery
   - irecovery -n

### A10 and A11 devices
- On A10 and A11, **you must disable your passcode while in the jailbroken state**.
  - On A10, this can be fixed in the future by implementing blackbird.
  - On A11, we don't have a SEP exploit yet.

# Prerequisites
#### Warning: You must install the Tips app from the App Store before running palera1n.
- A checkm8 vulnerable iOS device on iOS 15 (A8-A11)
  - The device must be on iOS 15.0-15.7.1
- Linux or macOS computer
  - Python 3 must be installed.

# How to use?
A better tutorial can be found [here](https://ios.cfw.guide/installing-palera1n).

1. Clone this repo with `git clone --recursive https://github.com/palera1n/palera1n && cd palera1n`
2. Run `./palera1n.sh --tweaks <your current iOS version>` (run with `sudo` if you're on linux)
   - [A10 and A11] Before running, **you must disable your passcode**.
   - Put your device in DFU mode before running.
3. Follow the steps on your screen.

# Repos

### Tweaks mode
All repos work when using tweaks mode because it uses normal Procursus and not rootless.

### Rootless 
Repos need to be updated for rootless, here are some that work currently:

- [Mineek's repo](https://mineek.github.io/repo) contains rootless Procursus packages
- The official [palera1n repo](https://repo.palera.in) contains miscellaneous packages

If you want to make a rootless repo, use the official [palera1n repo](https://github.com/palera1n/repo) for reference. Every deb should use the `iphoneos-arm64` architecture, and *nothing* should be on the rootfs. Everything should be in /var/jb.

# Credits
- [Nathan](https://github.com/verygenericname)
    - The ramdisk that dumps blobs, installs pogo to tips app, and duplicates rootfs is a slimmed down version of [SSHRD_Script](https://github.com/verygenericname/SSHRD_Script)
    - For modified [restored_external](https://github.com/verygenericname/sshrd_SSHRD_Script)
    - Also helped Mineek getting the kernel up and running and with the patches
    - Helping with adding multiple device support
    - Fixing issues relating to camera.. etc by switching to fsboot
    - [iBoot64Patcher fork](https://github.com/verygenericname/iBoot64Patcher)
- [Mineek](https://github.com/mineek)
    - For the patching and booting commands
    - Adding tweak support
    - For patchfinders for RELEASE kernels
    - [Kernel15Patcher](https://github.com/mineek/PongoOS/tree/iOS15/checkra1n/Kernel15Patcher)
    - [Kernel64Patcher](https://github.com/mineek/Kernel64Patcher)
- [Amy](https://github.com/elihwyma) for the [Pogo](https://github.com/elihwyma/Pogo) app
- [checkra1n](https://github.com/checkra1n) for the base of the kpf
- [nyuszika7h](https://github.com/nyuszika7h) for the script to help get into DFU
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing [bootstrap](https://github.com/ProcursusTeam/Procursus)
- [F121](https://github.com/F121Live) for helping test
- [m1sta](https://github.com/m1stadev) for [pyimg4](https://github.com/m1stadev/PyIMG4)
- [tihmstar](https://github.com/tihmstar) for [pzb](https://github.com/tihmstar/partialZipBrowser)/original [iBoot64Patcher](https://github.com/tihmstar/iBoot64Patcher)/original [liboffsetfinder64](https://github.com/tihmstar/liboffsetfinder64)/[img4tool](https://github.com/tihmstar/img4tool)
- [xerub](https://github.com/xerub) for [img4lib](https://github.com/xerub/img4lib) and [restored_external](https://github.com/xerub/sshrd) in the ramdisk
- [Cryptic](https://github.com/Cryptiiiic) for [iBoot64Patcher](https://github.com/Cryptiiiic/iBoot64Patcher) fork, and [liboffsetfinder64](https://github.com/Cryptiiiic/liboffsetfinder64) fork
- [libimobiledevice](https://github.com/libimobiledevice) for several tools used in this project (irecovery, ideviceenterrecovery etc), and [nikias](https://github.com/nikias) for keeping it up to date
- [Nick Chan](https://github.com/asdfugil) general help with patches.
- [Sam Bingner](https://github.com/sbingner) for [Substitute](https://github.com/sbingner/substitute)
- Serena for helping with boot ramdisk.
