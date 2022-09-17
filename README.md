# palera1n
iOS 15.0-15.4.1 semi-tethered checkm8 "jailbreak"

# What does this do?
It boots the device with AMFI patches, and installs Pogo by Amy to the Tips app.

**WARNING**: As of now, this is pretty unstable. On my A11 device, it has the deep sleep bug while booted with palera1n, and will kernel panic about a minute after being in sleep mode.

# How to use
1. Install libimobiledevice
    - It's needed for `ideviceenterrecovery` and `ideviceinfo`
2. Clone this repo with `git clone https://github.com/itsnebulalol/palera1n && cd palera1n`
3. Prepare your blob for the **current version** you're on
<!-- 4. Run `./palera1n.sh path/to/blob.shsh2 install`
    - \[A10+\] Before running, you **must** disable your passcode
    - If you want to start from DFU, run `./palera1n.sh path/to/blob.shsh2 --dfu <your iOS version here> install` -->
4. Run `./palera1n.sh path/to/blob.shsh2`
    - \[A10+\] Before running, you **must** disable your passcode
    - If you want to start from DFU, run `./palera1n.sh path/to/blob.shsh2 --dfu <your iOS version here>`
5. Make sure your device is in normal mode, if you didn't start from DFU
6. Follow the steps
    - Right now, getting into DFU is steps for A11, please suppliment the steps for your device
<!-- 7. Once your device reboots, run the script again, but without `install` -->
7. Install Pogo through TrollStore, then hit Install in the Pogo app!
    - You can get a Pogo IPA from [here](https://nightly.link/elihwyma/Pogo/workflows/build/main/Pogo.zip)
    - You should now see Sileo on your homescreen, enjoy!
    - You'll have to uicache in the Pogo app every reboot

# Credits
- [Nathan](https://github.com/verygenericname) for a lot of the code from SSHRD_Script
- [Mineek](https://github.com/mineek) for some of the patching and booting commands
- [Amy](https://github.com/elihwyma) for the Pogo app
- [nyuszika7h](https://github.com/nyuszika7h) for the script to get into DFU
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing bootstrap
