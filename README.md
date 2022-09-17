# palera1n
iOS 15.0-15.4.1 checkm8 "jailbreak"

# What does this do?
Currently, it just boots the device with AMFI patches. In the future, I'd want this to install the Pogo app automatically when it's public.

# How to use
1. Install libimobiledevice
    - It's needed for `ideviceenterrecovery` and `ideviceinfo`
2. Clone this repo with `git clone https://github.com/itsnebulalol/palera1n && cd palera1n`
3. Prepare your blob for the current version you're on
4. Run `./palera1n.sh path/to/blob.shsh2`
5. Make sure your device is in normal mode
6. Follow the steps
    - Right now, getting into DFU is steps for A11, please suppliment the steps for your device

# Credits
- [Nathan](https://github.com/verygenericname) for a lot of the code from SSHRD_Script
- [Mineek](https://github.com/mineek) for some of the patching and booting commands
- [Amy](https://github.com/elihwyma) for the Pogo app
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing bootstrap
