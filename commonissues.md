
  

# Common Issues

  

-  ## Table of Contents

1. GLIBC not found
2. "Booted device" but not booted
3. NewTerm not launching
4. palera1n/mineek repo not working
5. Procursus "killed 9"
6. App crashes on open
7. "Killed" issue (not "Killed: 9")
8. Error while loading shared libraries: libssl.so.1.1
9. Please reinstall Sileo via SSH
10. How to fix RocketBootstrap
11. Loader app not appearing **(most common)**
12. Your local changes would be overwritten by checkout
13. Could not connect to lockdownd
14. Package is in a very bad inconsistent state
15. Error installing bootstrap. Status: -1
16. jbinit DIED!
17. Daemons crashing on iOS 16.2+
18. Panics making loader not appear
19. libhooker wants to install?
20. Pressing “install” on each jb (don't do it)
21. --restorerootfs still keeps app icons??
22. Device boots out of DFU

  

## GLIBC not found

Add the universe repo by using the command below:
- `sudo apt-add-repository universe && sudo apt update`

Then, try again, and it should be fixed.

## "Booted device" but not booted
This may happen when the downloading and patching process is interrupted. Please run `./palera1n.sh clean` (use with `sudo` if on Linux), then try again.
If that doesn't fix it, it may be caused by an update from the Procursus repo. The quickest way to fix it is `./palera1n.sh --restorerootfs`. Alternatively, you can manually restore `/usr/libexec/dirs_cleaner` from the rootfs snapshot using the SSHRD script.

## NewTerm not launching
Install NewTerm 2 from [https://apt.itsnebula.net/](https://apt.itsnebula.net/ "https://apt.itsnebula.net/") or get NewTerm3 beta.

## palera1n/mineek repo not working
[https://repo.palera.in/](https://repo.palera.in/ "https://repo.palera.in/") and [https://mineek.github.io/repo](https://mineek.github.io/repo "https://mineek.github.io/repo") are supposed to be used on rootless jailbreaks, not with jailbreaks with fakefs or tethered that have root access.

## Procursus "killed 9"
Binaries will need to be resigned by the Procursus Team to fix killed 9, in the meantime, use the palera1n strap repo. you can install it from [nebula’s repo](https://apt.itsnebula.net).

## App crashes on open
- `ldid -s /Applications/<appname>.app`
	- Don't include the <>

## "Killed" issue (*not* "Killed: 9")
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

## How to fix RocketBootstrap
1. Remove the Odyssey repo if you have it (to make sure you don't mistakenly install full libhooker, which *does not work* with palera1n)
2. Add the [Havoc repo](https://havoc.app/)
3. Install `libhooker-shim` (if it wants to remove Substitute, you've done something wrong)
4. Get this deb: [https://repo.theodyssey.dev/debs/rocketbootstrap_1.1.0~libhooker2-iphoneos-arm.deb](https://repo.theodyssey.dev/debs/rocketbootstrap_1.1.0~libhooker2-iphoneos-arm.deb "https://repo.theodyssey.dev/debs/rocketbootstrap_1.1.0~libhooker2-iphoneos-arm.deb")
5. Open it in Sileo or Filza, and install it.
6. Reboot userspace (`launchctl reboot userspace`)

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

## Error installing bootstrap. Status: -1
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

## Pressing “install” on each jb
**DO NOT DO THIS.** It will likely break your jailbreak install. Instead, press the gear icon, and then press Do All.

## --restorerootfs still keeps app icons??
There's an issue with restoring rootfs where it doesn’t uicache. **There is no need to worry**; this happens on a couple of other jailbreaks and serves no harm to the user or device.

Example image:
![image depicting blank icons for apps](https://media.discordapp.net/attachments/1028693596469207191/1065709922207150080/1A4B5107-DCFA-42E6-AC2D-1106CD854FC2.jpg?width=830&height=434)

## Device boots out of DFU
Make sure to use a USB-A cable, and enter recovery mode first before entering DFU (you can use `./palera1n.sh dfuhelper` for this - use `sudo` if on Linux).