
  

# Common Issues

  

## Table of Contents
  
1. [GLIBC not found](#glibc-not-found)
2. ["Booted device" but not booted](#booted-device-but-not-booted)
3. [NewTerm not launching](#newterm-not-launching)
4. [palera1n/mineek repo not working](#palera1nmineek-repo-not-working)
5. [Procursus "killed 9"](#procursus-killed-9)
6. [App crashes on open](#app-crashes-on-open)
7. ["Killed" issue (*not* "Killed: 9")](#killed-issue-not-killed-9)
8. [Error while loading shared libraries: libssl.so.1.1](#error-while-loading-shared-libraries-libsslso11)
9. [Please reinstall Sileo via SSH](#please-reinstall-sileo-via-ssh)
10. [Loader app not appearing](#loader-app-not-appearing)
11. [Your local changes would be overwritten by checkout](#your-local-changes-would-be-overwritten-by-checkout)
12. [Could not connect to lockdownd](#could-not-connect-to-lockdownd)
13. [Package is in a very bad inconsistent state](#package-is-in-a-very-bad-inconsistent-state)
14. [Error installing bootstrap. Status: -1](#error-installing-bootstrap-status--1)
15. [jbinit DIED!](#jbinit-died)
16. [Daemons crashing on iOS 16.2+](#daemons-crashing-on-ios-162)
17. [Panics making loader not appear](#panics-making-loader-not-appear)
18. [libhooker wants to install?](#libhooker-wants-to-install)
19. [Pressing "install" on each jb](#pressing-install-on-each-jb)
20. [--restorerootfs still keeps app icons??](#--restorerootfs-still-keeps-app-icons)
21. [Device boots out of DFU](#device-boots-out-of-dfu)
22. [How to fix RocketBootstrap](#how-to-fix-rocketbootstrap)
23. [SEP Panic: :skg /skgs](#sep-panic-skg-skgs)
24. [Cannot download apps from the App Store](#cannot-download-apps-from-the-app-store)
25. [Snowboard theming incorrectly](#snowboard-theming-incorrectly)
26. [End-of-central-directory signature not found](#end-of-central-directory-signature-not-found)

## GLIBC not found

Add the universe repo by using the command below:
- `sudo apt-add-repository universe && sudo apt update`

Then, try again, and it should be fixed.

## \"Booted device\" but not booted
This may happen when the downloading and patching process is interrupted. Please run `./palera1n.sh clean` (use with `sudo` if on Linux), then try again.
If that doesn't fix it, it may be caused by an update from the Procursus repo. The quickest way to fix it is `./palera1n.sh --restorerootfs`. Alternatively, you can manually restore `/usr/libexec/dirs_cleaner` from the rootfs snapshot using the SSHRD script.

## NewTerm not launching
Install NewTerm 2 from [https://apt.itsnebula.net/](https://apt.itsnebula.net/ "https://apt.itsnebula.net/") or get NewTerm3 beta.

## palera1n\/mineek repo not working
[https://repo.palera.in/](https://repo.palera.in/ "https://repo.palera.in/") and [https://mineek.github.io/repo](https://mineek.github.io/repo "https://mineek.github.io/repo") are supposed to be used on rootless jailbreaks, not with any rootful jailbreaks with fakefs or tethered that have root access.

## Procursus \"killed 9\"
Binaries will need to be resigned by the Procursus Team to fix Killed: 9. In the meantime, use the palera1n strap repo. you can install it from [nebula’s repo](https://apt.itsnebula.net).

## App crashes on open
- `ldid -s /Applications/<appname>.app`
	- Don't include the <>

## \"Killed\" issue \(*not* \"Killed: 9\"\)
You ran out of RAM on the Linux Live CD.

Ways to fix the issue, ordered by which to try first: 
1. Close some apps, like Discord
2. Attempt a shallow clone, `git clone --depth=1` in place of `git clone`
3. Clone palera1n onto persistent storage[note 1]
4. Install Linux onto your computer.

Note 1: How to use persistent storage from Linux Live, with terminal
 1. If you have already cloned palera1n, please delete it! Usually this can be done with `sudo rm -rf ~/palera1n` (assuming palera1n cloned into home directory)
 2. You may want to install ntfs-3g first, on Ubuntu this can be done by running `sudo apt install ntfs-3g`
 3. Run `sudo lsblk` to list your disks
 4. Locate the persistent storage you want to use, for example a Windows C: drive would be of type "ntfs" and at least 10 GB in size, take a note of the device name, which starts with /dev
 5. Mount disk onto `/mnt` *(example, please don’t paste as is)*: `sudo mount -t ntfs /dev/sda3 /mnt`
 6. Change the working directory into `/mnt`: `cd /mnt`
 7. Try to clone palera1n again

Note 2: Most Linux Installers will set  up swap areas automatically during the install if you use guided partitioning (if such option exists)
In order to create a 2GB swap area as root:
 1. Run `dd if=/dev/zero of=/swapfile bs=1M count=2048`
 2. Run `chmod 600 /swapfile`
 3. Format by running `mkswap /swapfile`
 4. Open /etc/fstab in an editor like `nano`, then add a line that is `/swapfile none swap sw 0 0`
 5. Run `swapon /swapfile`

## Error while loading shared libraries: libssl.so.1.1
You can workaround this by installing OpenSSL 1.1 using the command(s) below (on systems with dpkg):
- `curl -LO http://nz2.archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb`  
- `sudo dpkg -i libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb`

## Please reinstall Sileo via SSH
If you have AutoSign installed and you reinstall/update Sileo, you'll get the error message shown in the screenshot. Here's how to fix it.

1. Get access to a terminal by either using SSH or NewTerm
2. Run `sudo apt remove autosign`. Unless you changed it, the password is `alpine`
3. Once it's done, run `sudo apt reinstall org.coolstar.sileo`
4. Try running Sileo again. Feel free to upgrade it if that was what you were planning to do.
5. Install AutoSign again

It should be fixed after these steps.

Example image: 
![image with an error onscreen stating: "Sileo was unable to acquire root permissions. Please reinstall Sileo using SSH."](https://media.discordapp.net/attachments/1028693596469207191/1051879461093650442/1F92489D-5CA5-4E0C-A1FE-CED814FB0089.png?width=244&height=434)


**Note**: Some tweaks (e.g. Watusi) are known to not work with this method.

## Loader app not appearing
restorerootfs and rejailbreak with `--restorerootfs` at the end of your previous command. If neither of these help, make sure you're waiting enough time (15-30 seconds).
On iPads, the loader may not show up. You can try to open it using [this shortcut](https://www.icloud.com/shortcuts/8cd5f489c8854ee0ab9ee38f2e62f87d). If other apps don’t appear along side the loader, try installing TrollHelper from [Havoc](https://havoc.app), and install TrollStore.

## Your local changes would be overwritten by checkout
Run the following commands, and then try again:
- `cd ramdisk`
- `git stash`
- `git stash drop`
- `cd ..`

## Could not connect to lockdownd
Try the following steps to fix the issue:
- Unplug and replug your device
- Run `idevicepair pair` (use `sudo` if on Linux)
- Trust the computer on your device if needed
- Run `idevicepair pair` again (use sudo if on Linux)

Alternatively, you can manually [enter recovery mode](https://support.apple.com/en-us/HT201263 "enter recovery mode") before starting palera1n.

## Package is in a very bad inconsistent state
You can fix this by running this command in a terminal:
- `sudo dpkg -r --force-remove-reinstreq <PACKAGE_NAME>`
	-	Don't include the <>

## Error installing bootstrap. Status: \-1
You're not jailbroken. Sideloading the loader app on its own **will not work**.

Please run the palera1n script on your computer to jailbreak your device.

## jbinit DIED!
Your device may get stuck on a verbose boot screen, and if you look closely you'll see a "jbinit DIED!" error near the top.

The simplest way to fix this is `rm -rf blobs` (use `sudo` if on Linux), then force reboot your device and try to jailbreak again.

**For advanced users**:
You can also try re-copying the `other/rootfs` files manually to the device using SSHRD.

## Daemons crashing on iOS 16.2+
Substitute may cause daemons to crash on iOS 16.2 and above. It's recommended to downgrade to iOS 16.1.2 or below for now. Or you can try using ElleKit instead, but note that some tweaks don't yet work with it.

## Panics making loader not appear
You may be encountering some issue related to panics and the loader “not appearing”, on A10+ make sure you have your passcode **disabled** before jailbreaking.

## libhooker wants to install?
**Remove the Chimera and Odyssey repo immediately!**

These repos are not meant to be used with palera1n and are able to break your jailbreak if you install anything from them.

## Pressing \"install\" on each jb
**DO NOT DO THIS.** It resets your package lists and will likely break your jailbreak install eventually. Instead, press the gear icon, and then press Do All.

## --restorerootfs still keeps app icons??
There's an issue with restoring rootfs where it doesn’t uicache. **There is no need to worry**; this happens on a couple of other jailbreaks and serves no harm to the user or device.

Example image:
![image depicting blank icons for apps](https://media.discordapp.net/attachments/1028693596469207191/1065709922207150080/1A4B5107-DCFA-42E6-AC2D-1106CD854FC2.jpg?width=830&height=434)

## Device boots out of DFU
Make sure to use a USB-A cable, and enter recovery mode first before entering DFU (you can use `./palera1n.sh dfuhelper` for this - use `sudo` if on Linux).

## How to fix RocketBootstrap
1. Uninstall any existing version of RocketBootstrap you have installed
2. Remove the Odyssey repo if you have it
3. Install RocketBootstrap from [this repo](https://rpetri.ch/repo).
4. Install RocketBootstrapFix from [this repo](https://repo.alexia.lol/).

This is an updated fix that should work with more tweaks than the Odyssey version.

## SEP Panic: :skg /skgs
This happens due to having a passcode set on A10-A11 devices when jailbreaking (or having previously set a passcode on iOS 16, even if it's currently turned off).

If you are on A10, use https://github.com/guacaplushy/checkp4le.
<details>
<summary>Why?</summary>
checkp4le boots with checkra1n using a custom kernel patchfinder, which allows SEP to be re-enabled on A10
</details>

If you are on A11
- iOS 15
  - Turn off your passcode and try jailbreaking again.
- iOS 16
  - Turn off your passcode, backup your device, erase from settings or restore from iTunes/Finder, and then restore the backup. Then try jailbreaking again.

If you don't want to use checkp4le and just want to have a passcode, or if you're on A11, you can use FakePass from [this repo](https://repo.alexia.lol/) to have a passcode. However, this will only work in a jailbroken state (it can be bypassed by simply rebooting the device).

## Cannot download apps from the App Store
Install Choicy from [this repo](https://opa334.github.io) and disable tweak injection into the App Store.

## Snowboard theming incorrectly
You may be encountering an issue with Snowboard not theming correctly or not theming at all. To fix this, make sure you have Snowboard from [SparkDev's repo](https://sparkdev.me).

Example image:
![image depicting sb issue](https://media.discordapp.net/attachments/1028693596469207191/1070014279639650345/image.png)

## End-of-central-directory signature not found
If the unzip error message it cannot find "`palera1n.zip(.ZIP, period.)`", you are running an outdated version of palera1n and need to update using `git pull`. If this doesn't work, reclone the palera1n repository by running `cd .. && sudo rm -rf palera1n && sudo git clone --recursive --depth=1 --shallow-submodules https://github.com/palera1n/palera1n && cd palera1n`. 

Otherwise, this error most likely indicates a problem with your internet connection, and you simply need to try running palera1n again.

<br>

### If none of these solve your issue, please join the [Discord server](https://dsc.gg/palera1n).
