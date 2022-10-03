# palera1n
iOS 15.0-15.3.1 **work in progress, tethered** checkm8 "jailbreak" (TWEAKS BRANCH)

# What does this do?
It boots the device with AMFI patches. On first run, it'll boot a ramdisk which dumps your onboard blob, and installs Sileo and Substitute.

**WARNING**: On A11, it has the deep sleep bug while booted with palera1n, and will kernel panic, or just not wake up until force rebooted, about a minute after being in sleep mode.

**WARNING 2**: I am NOT responsible for any data loss. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device is stuck in recovery, please run `futurerestore --exit-recovery`, or use `irecovery -n`. Using this on iOS 16 has a higher chance of bootlooping you.

On A10 and A11, you **must disable your passcode while in the jailbroken state**. On A10, this can be fixed in the future by implementing blackbird. On A11, we don't have a SEP exploit yet. It may also **break camera while in the jailbroken state**.

# Linux issues
Linux has some weird usbmuxd issues. We have tried our best to fix them, but there stil are issues. We highly recommend to compile and install [usbmuxd2](https://github.com/tihmstar/usbmuxd2).

Stop making issues about Linux not being able to connect, we are aware. This includes being stuck on waiting for ramdisk to finish booting.

# Prerequisites
1. checkm8 vulnerable iOS device on iOS 15 (A8X-A11)
2. Linux or macOS computer
    - Python 3 is required
3. iOS 15.0-15.3.1
4. A brain
    - Remember, this is mainly for developers.

# How to use
1. Clone this repo with `git clone --recursive https://github.com/palera1n/palera1n && cd palera1n`
2. Run `./palera1n.sh --tweaks <ios version youre on atm>`
    - \[A10+\] Before running, you **must** disable your passcode
    - Put you're device in DFU Mode first.
3. Follow the steps
    - Right now, getting into DFU is steps for A11, please suppliment the steps for your device

# Repos
All repos work with the tweaks branch because it uses normal procursus and not rootless.

# Credits
- [Nathan](https://github.com/verygenericname) for a lot of the code from SSHRD_Script
    - The ramdisk that dumps blobs is a slimmed down version of SSHRD_Script
<<<<<<< HEAD
    - Also helped Mineek getting the kernel up and running and with the patches.
- [Mineek](https://github.com/mineek) for the patching and booting commands, and adding tweak support.
||||||| 554eb0d
    - They also helped me ( mineek ) getting the kernel up and running and with the patches.
- [Mineek](https://github.com/mineek) for the patching and booting commands, and adding tweak support.
=======
    - They also helped me ( mineek ) getting the kernel up and running and with the patches.
    - Helping with adding multiple device support.
- [Mineek](https://github.com/mineek)
    - for the patching and booting commands
    - adding tweak support.
>>>>>>> 88c9f50f60efafbccb50c7995f94d9971532d0f8
- [Amy](https://github.com/elihwyma) for the Pogo app
- [nyuszika7h](https://github.com/nyuszika7h) for the script to help get into DFU
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing bootstrap
