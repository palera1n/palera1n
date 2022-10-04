# This is a work in progress.

Read this throughly, feel free to ask questions, know the risks. If you want to ask questions, either:

1. Ask in r/jailbreak Discord #palera1n channel
2. Ask in Permasigner Discord #palera1n channel
3. Open a GitHub issue

Please, please, please, provide necessary info:

- iOS version and device (eg. iPhone 7+ 15.1, iPhone 6s 15.3.1)
- Computer's OS and version (eg. Ubuntu 22.04, macOS 13.0)
- The command you ran
- Debug logs with `--debug`

**DO NOT** harass tweak devs if tweaks don't work. Refer to [here](https://github.com/itsnebulalol/ios15-tweaks) for compatiblity.

# palera1n

iOS 15.0-15.3.1 **work in progress, tethered** checkm8 "jailbreak" (TWEAKS BRANCH)

# What does this do?

It boots the device with AMFI patches. On first run, it'll boot a ramdisk which dumps your onboard blob, and installs Sileo and Substitute.

**WARNING 1**: I am NOT responsible for any data loss. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device is stuck in recovery, please run `futurerestore --exit-recovery`, or use `irecovery -n`. Using this on iOS 16 has a higher chance of bootlooping you.

On A10 and A11, you **must disable your passcode while in the jailbroken state**. On A10, this can be fixed in the future by implementing blackbird. On A11, we don't have a SEP exploit yet.

# Linux issues
Linux has some weird usbmuxd issues. We have tried our best to fix them, but there stil are issues. We highly recommend to compile and install [usbmuxd2](https://github.com/tihmstar/usbmuxd2).

Stop making issues about Linux not being able to connect, we are aware. This includes being stuck on waiting for ramdisk to finish booting.

# Prerequisites
1. checkm8 vulnerable iOS device on iOS 15 (A8X-A11)
    - You must install the Tips app from the App Store before running the script
2. Linux or macOS computer
    - Python 3 is required
3. iOS 15.0-15.3.1
4. A brain
    - Remember, this is mainly for developers.

# How to use
1. Clone this repo with `git clone -b tweaks --recursive https://github.com/palera1n/palera1n && cd palera1n`
2. Run `./palera1n.sh --tweaks <ios version youre on atm>`
    - \[A10+\] Before running, you **must** disable your passcode
    - Put your device in DFU Mode before running.
3. Follow the steps

# Repos
All repos work with the tweaks branch because it uses normal Procursus and not rootless.

# Credits
- [Nathan](https://github.com/verygenericname)
    - The ramdisk that dumps blobs is a slimmed down version of SSHRD_Script
    - Also helped Mineek getting the kernel up and running and with the patches
    - Helping with adding multiple device support
- [Mineek](https://github.com/mineek)
    - For the patching and booting commands
    - Adding tweak support
- [Amy](https://github.com/elihwyma) for the Pogo app
- [nyuszika7h](https://github.com/nyuszika7h) for the script to help get into DFU
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing bootstrap
- [F121](https://github.com/F121Live) for helping test
- [tihmstar](https://github.com/tihmstar) for pzb/original iBoot64Patcher/img4tool
- [xerub](https://github.com/xerub) for img4lib and restored_external in the ramdisk
- [Cryptic](https://github.com/Cryptiiiic) for iBoot64Patcher fork
