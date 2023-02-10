
# Common Issues

  

## Table of Contents
  
1. ["Booted device" but not booted](#booted-device-but-not-booted)
2. [NewTerm not launching](#newterm-not-launching)
3. [palera1n/mineek repo not working](#palera1nmineek-repo-not-working)
4. [Procursus "killed 9"](#procursus-killed-9)
5. [App crashes on open](#app-crashes-on-open)
6. ["Killed" issue (*not* "Killed: 9")](#killed-issue-not-killed-9)
7. [Please reinstall Sileo via SSH](#please-reinstall-sileo-via-ssh)
8. [Loader app not appearing](#loader-app-not-appearing)
9. [Your local changes would be overwritten by checkout](#your-local-changes-would-be-overwritten-by-checkout)
10. [Could not connect to lockdownd](#could-not-connect-to-lockdownd)
11. [Package is in a very bad inconsistent state](#package-is-in-a-very-bad-inconsistent-state)
12. [Error installing bootstrap. Status: -1](#error-installing-bootstrap-status--1)
13. [jbinit DIED!](#jbinit-died)
14. [Daemons crashing on iOS 16.2+](#daemons-crashing-on-ios-162)
15. [Panics making loader not appear](#panics-making-loader-not-appear)
16. [libhooker wants to install?](#libhooker-wants-to-install)
17. [Pressing "install" on each jb](#pressing-install-on-each-jb)
18. [--restorerootfs still keeps app icons??](#--restorerootfs-still-keeps-app-icons)
19. [Device boots out of DFU](#device-boots-out-of-dfu)
20. [How to fix RocketBootstrap](#how-to-fix-rocketbootstrap)
21. [SEP Panic: :skg /skgs](#sep-panic-skg-skgs)
22. [Cannot download apps from the App Store](#cannot-download-apps-from-the-app-store)
23. [Snowboard theming incorrectly](#snowboard-theming-incorrectly)
24. [End-of-central-directory signature not found](#end-of-central-directory-signature-not-found)
25. [Filza crashing on launch](#filza-crashing-on-launch)
26. [dpkg error: Read-only file system](#dpkg-error-read-only-file-system)
27. [Library not loaded: /usr/lib/libSystem.B.dylib](#library-not-loaded-usrliblibsystembdylib)
28. [Stuck at waiting for network](#stuck-at-waiting-for-network)
29. [No space left on device](#no-space-left-on-device)
30. [Found the USB handle followed by an error occurred](#found-the-usb-handle-followed-by-an-error-occurred)
31. [pip error: legacy-install-failure](#pip-error-legacy-install-failure)

## \"Booted device\" but not booted
This may happen when the downloading and patching process is interrupted. Please run `./palera1n.sh clean` (use with `sudo` if on Linux), then try again.
If that doesn't fix it, it may be caused by an update from the Procursus repo. The quickest way to fix it is to [restore rootfs](https://ios.cfw.guide/removing-palera1n/). Alternatively, you can manually restore `/usr/libexec/dirs_cleaner` from the rootfs snapshot using the SSHRD script.

## NewTerm not launching
Install NewTerm 2 from [https://apt.itsnebula.net/](https://apt.itsnebula.net/ "https://apt.itsnebula.net/") or get NewTerm3 beta.

## palera1n\/mineek repo not working
[https://repo.palera.in/](https://repo.palera.in/ "https://repo.palera.in/") and [https://mineek.github.io/repo](https://mineek.github.io/repo "https://mineek.github.io/repo") are supposed to be used on rootless jailbreaks, not with any rootful jailbreaks with fakefs or tethered that have root access.

## Procursus \"killed 9\"
Binaries will need to be resigned by the Procursus Team to fix Killed: 9. In the meantime, use the palera1n strap repo. you can install it from [nebula’s repo](https://apt.itsnebula.net).

## App crashes on open
Refer to [this](#how-to-fix-rocketbootstrap) first.

Otherwise, run this in NewTerm or SSH
- `ldid -s /Applications/<appname>.app`
	- Don't include the <>

## \"Killed\" issue \(*not* \"Killed: 9\"\)
You ran out of RAM on the Linux Live CD.

Ways to fix the issue, ordered by which to try first: 
1. Close some apps, like Discord
2. Attempt a shallow clone, `git clone --depth=1` in place of `git clone`
3. Clone palera1n onto persistent storage[note 1]
4. Install Linux onto your computer.

**Note 1: How to use persistent storage from Linux Live, with terminal**
 1. If you have already cloned palera1n, please delete it! Usually this can be done with `sudo rm -rf ~/palera1n` (assuming palera1n cloned into home directory)
 2. You may want to install ntfs-3g first, on Ubuntu this can be done by running `sudo apt install ntfs-3g`
 3. Run `sudo lsblk` to list your disks
 4. Locate the persistent storage you want to use, for example a Windows C: drive would be of type "ntfs" and at least 10 GB in size, take a note of the device name, which starts with /dev
 5. Mount disk onto `/mnt` *(example, please don’t paste as is)*: `sudo mount -t ntfs /dev/sda3 /mnt`
 6. Change the working directory into `/mnt`: `cd /mnt`
 7. Try to clone palera1n again

**Note 2: Most Linux Installers will set  up swap areas automatically during the install if you use guided partitioning (if such option exists)**
In order to create a 2GB swap area as root:
 1. Run `dd if=/dev/zero of=/swapfile bs=1M count=2048`
 2. Run `chmod 600 /swapfile`
 3. Format by running `mkswap /swapfile`
 4. Open /etc/fstab in an editor like `nano`, then add a line that is `/swapfile none swap sw 0 0`
 5. Run `swapon /swapfile`

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
Try these steps:
1. Check if [this shortcut](https://www.icloud.com/shortcuts/8cd5f489c8854ee0ab9ee38f2e62f87d) opens it
2. Check for any panic logs by going to
	1. Settings
	2. Privacy
	3. Analytics & Improvements
	4. Analytics Data
	- If there are any, send the **latest** one that starts with `panic-full`
		- **Please do not send any other device log**
3. If these steps above don't work, [restore rootfs](https://ios.cfw.guide/removing-palera1n/) and rejailbreak

If none of these help, make sure you're waiting enough time [for the respring] (15-30 seconds).

On iPads, if other apps don’t appear along side the loader, try installing TrollStore Helper from the [Havoc repo](https://havoc.app) and use it to install TrollStore.

## Your local changes would be overwritten by checkout
Run the following commands, and then try again:
- `cd ramdisk`
- `git stash`
- `git stash drop`
- `cd ..`

## Could not connect to lockdownd
**Mux error (-8)**
- Unplug and plug your device back in

**Pairing dialog response pending (-19)**
- Accept the prompt to trust the computer on your device.

**Invalid HostID (-21)**
1. Run `idevicepair pair` (use `sudo` on Linux)
2. Accept the prompt to trust the computer on your device if needed
3. Run `idevicepair pair` again (use `sudo` on Linux)
- Alternatively, try entering [recovery mode manually](https://support.apple.com/en-us/HT201263 "enter recovery mode") before jailbreaking.

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
Substitute may cause daemons to crash on iOS 16.2 and above. It's recommended to downgrade to iOS 16.1.2 or below for now.
- If you don't have blobs saved, you can downgrade to 15.6 RC and delay OTA to 16.1.2. This expires in a month.

Or you can try using ElleKit instead, but note that some tweaks don't yet work with it.
<details>
<summary>How to Install ElleKit</summary>
<code>sudo rmdir /usr/lib/TweakInject
sudo ln -s /Library/MobileSubstrate/DynamicLibraries /usr/lib/TweakInject
sudo mv /Library/MobileSubstrate/DynamicLibraries/MobileSafety.dylib{,.disabled}
sudo mkdir -p /Library/Frameworks/CydiaSubstrate.framework
sudo ln -s /usr/lib/libsubstrate.dylib /Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate
sudo /usr/libexec/ellekit/loader
sudo mkdir -p /etc/rc.d
sudo ln -s /usr/libexec/ellekit/loader /etc/rc.d/ellekit-loader
launchctl reboot userspace</code>
	
Then launch daemons in loader.
</details>

## Panics making loader not appear
You may be encountering some issue related to panics and the loader “not appearing”, on A10+ make sure you have your passcode **disabled** before jailbreaking.

## libhooker wants to install?
**Remove the Chimera and Odyssey repo immediately!**

These repos are not meant to be used with palera1n and are able to break your jailbreak if you install anything from them.

## Pressing \"install\" on each jb
**DO NOT DO THIS.** It resets your package lists and will likely break your jailbreak install eventually. Instead, press the gear icon, and then press Do All.

If you managed to mess up your jailbreak this way, [restore rootfs](https://ios.cfw.guide/removing-palera1n/).

## --restorerootfs still keeps app icons??
There's an issue with restoring rootfs where it doesn’t uicache. **There is no need to worry**; this happens on a couple of other jailbreaks and serves no harm to the user or device.

Example image:
![image depicting blank icons for apps](https://media.discordapp.net/attachments/1028693596469207191/1065709922207150080/1A4B5107-DCFA-42E6-AC2D-1106CD854FC2.jpg?width=830&height=434)

## Device boots out of DFU
Make sure to use a USB-A cable, and follow one of the methods below to enter DFU.

**Method 1**
1. Make sure your device is in either normal or recovery mode.
2. Connect your device to your computer.
3. Run `./palera1n.sh dfuhelper` (use `sudo` if on Linux).
4. Follow the on-screen instructions.

**Method 2**
1. Power off your device.
2. Connect it to your computer or a charger.
3. As soon as the Apple logo comes on, start doing the [DFU mode sequence](https://help.ifixit.com/article/108-dfu-restore).

## How to fix RocketBootstrap
If you have Ryan Petrich's RocketBootstrap installed on iOS 15 or above, it may cause nearly every app to crash or even respring loops, especially when Cephei is also installed. To fix this, follow the steps below.

1. Uninstall any existing version of RocketBootstrap you have installed
2. Remove the Odyssey repo if you have it
3. Install RocketBootstrap from [this repo](https://rpetri.ch/repo).
4. Install RocketBootstrapFix from [this repo](https://repo.alexia.lol/).

This is an updated fix that should work with more tweaks than the Odyssey version.

## SEP Panic: :skg /skgs
This happens due to having a passcode set on A10-A11 devices when jailbreaking (or having previously set a passcode on iOS 16, even if it's currently turned off).

If you are on A10, use [checkp4le](https://github.com/guacaplushy/checkp4le).
<details>
<summary>Why?</summary>
checkp4le boots with checkra1n using a custom kernel patchfinder, which allows SEP to be re-enabled on A10
</details>

If you are on A11
- iOS 15
  - Turn off your passcode and try jailbreaking again.
- iOS 16
  1. Turn off your passcode
  2. Backup your device
  3. Erase from settings or restore from iTunes/Finder
  4. Restore the backup. 
  5. Then try jailbreaking again.

If you don't want to use checkp4le and just want to have a passcode, or if you're on A11, you can use FakePass from [this repo](https://repo.alexia.lol/) to have a passcode. However, this will only work in a jailbroken state (it can be bypassed by simply rebooting the device).

## Cannot download apps from the App Store
Install Choicy from [this repo](https://opa334.github.io) and disable tweak injection into the App Store.

Alternatively, disable tweak injection globally in the Substitute app and respring. You can re-enable it after downloading the apps you wanted.

## Snowboard theming incorrectly
You may be encountering an issue with Snowboard not theming correctly or not theming at all. To fix this, make sure you have Snowboard from [SparkDev's repo](https://sparkdev.me).

Example image:
![image depicting sb issue](https://media.discordapp.net/attachments/1028693596469207191/1070014279639650345/image.png)

## End-of-central-directory signature not found
If the unzip error message it cannot find "`palera1n.zip(.ZIP, period.)`", you are running an outdated version of palera1n and need to update using `git pull`. If this doesn't work, reclone the palera1n repository by running `cd .. && sudo rm -rf palera1n && sudo git clone --recursive --depth=1 --shallow-submodules https://github.com/palera1n/palera1n && cd palera1n`. 

Otherwise, this error most likely indicates a problem with your internet connection, and you simply need to try running palera1n again.

## Filza crashing on launch
Install the latest version of Filza from the [TIGI Software repo](https://www.tigisoftware.com/repo). **The version on BigBoss is outdated**. You do not need AutoSign or FilzaFixer anymore.

## dpkg error: Read-only file system
  - Open the palera1n loader app
  - Press the gear icon
  - Remount R/W

Then try again.

## Library not loaded: /usr/lib/libSystem.B.dylib
This usually means the fakefs wasn't created properly. It might be left over from a restore. [Restore rootfs](https://ios.cfw.guide/removing-palera1n/) and try jailbreaking again.

## Stuck at `waiting for network`
This is usually caused by a network issue.

1. Make sure you're on the latest commit (`git pull`)
2. Make sure you can access [this site](https://static.palera.in/)

If you can access it but the script still doesn't work, your network may be unstable or certain outgoing requests may be blocked. If you cannot access it, this may mean that your network is blocking it, or it's currently down.

## No space left on device
This means that either your iDevice or your computer does not have enough storage.

If you're live booting Linux and have a low amount of RAM, follow [this](#killed-issue-not-killed-9).

If you're using semi-tethered and have less than 10GB available on the iDevice, this may be the issue. Please [restore rootfs](https://ios.cfw.guide/removing-palera1n/) and clear some space on your device. Usually if this is the issue, the console is spammed many times with `No space left on device` or `File or directory not found.`

## Found the USB handle followed by an error occurred
This will happen if the device is in a bad DFU state, which could randomly happen, or if you held the 2nd DFU button for too long (volume down on iPhone X, for example).

Reboot the device and rerun the script, and make sure to follow the DFU helper exactly, and let go as soon as it says it found the device in DFU.

## pip error: legacy-install-failure
- **Ubuntu/Debian**
  	- `sudo apt install python3-dev`
- **Fedora**
  	- `sudo dnf install python3-devel`
- **openSUSE**
  	- `sudo zypper install python3-devel`
- **macOS**
	- Install the Xcode Command Line Tools by running
		- `xcode-select --install`

	- If it still doesn't work, try this command:
		- `ARCHFLAGS="-arch $(uname -m)" python3 -m pip install --compile pyimg4`

<br>

### If none of these solve your issue, please join the [Discord server](https://dsc.gg/palera1n).
