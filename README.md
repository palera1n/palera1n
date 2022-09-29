# palera1n
iOS 15.0-15.7 **work in progress** semi-tethered checkm8 "jailbreak"

# What does this do?
It boots the device with AMFI patches. On first run, it'll boot a ramdisk which dumps your onboard blob, and installs the loader app (Pogo by Amy) to Tips. This is mainly a **developer** jailbreak. There is **no tweak injection yet**.

**WARNING**: On A11, it has the deep sleep bug while booted with palera1n, and will kernel panic, or just not wake up until force rebooted, about a minute after being in sleep mode.

**WARNING 2**: I am NOT responsible for any data loss. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device is stuck in recovery, please run `futurerestore --exit-recovery`, or use `irecovery -n`. Using this on iOS 16 has a higher chance of bootlooping you.

On A10 and A11, you **must disable your passcode while in the jailbroken state**. On A10, this can be fixed in the future by implementing blackbird. On A11, we don't have a SEP exploit yet. It may also **break camera while in the jailbroken state**.

# Linux issues
Linux has some weird usbmuxd issues. We have tried our best to fix them, but there stil are issues. We highly recommend to compile and install [usbmuxd2](https://github.com/tihmstar/usbmuxd2).

Stop making issues about Linux not being able to connect, we are aware.

# Prerequisites
1. checkm8 vulnerable iOS device on iOS 15 (A8X-A11)
2. Linux or macOS computer
    - Python 3 is required

# How to use
1. Clone this repo with `git clone --recursive https://github.com/palera1n/palera1n && cd palera1n`
2. Run `./palera1n.sh`
    - \[A10+\] Before running, you **must** disable your passcode
    - If you want to start from DFU, run `./palera1n.sh --dfu <your iOS version here>`
3. Make sure your device is in normal mode, if you didn't start from DFU
4. Follow the steps
    - Right now, getting into DFU is steps for A11, please suppliment the steps for your device
5. Open the Tips app, and click install!
    - If Pogo didn't install to Tips for some reason, you can get a Pogo IPA from [here](https://nightly.link/elihwyma/Pogo/workflows/build/main/Pogo.zip) to install with TrollStore
    - You should now see Sileo on your homescreen, enjoy!
    - You'll have to uicache in the Pogo app every reboot

# Repos
Known repos are as followed:
- https://mineek.github.io/repo
    - This repo has a lot of Procursus rootless packages
- https://repo.palera.in
    - A ton of miscellaneous packages
- https://beta.anamy.gay - main - main
    - Contains Sileo Nightly
    - This is a **dist** repo

Make a repo:
- Release and package architecture should be `iphoneos-arm64`
- Sign your repos!!!

Use the [palera1n repo](https://github.com/palera1n/repo) as an example

# Credits
- [Nathan](https://github.com/verygenericname) for a lot of the code from SSHRD_Script
    - The ramdisk that dumps blobs is a slimmed down version of SSHRD_Script
- [Mineek](https://github.com/mineek) for the patching and booting commands
- [Amy](https://github.com/elihwyma) for the Pogo app
- [nyuszika7h](https://github.com/nyuszika7h) for the script to help get into DFU
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing bootstrap
