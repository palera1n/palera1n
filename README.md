<img align="left" height="120" src="https://cdn.discordapp.com/attachments/1017854329887129611/1073858292159352862/thing.png" alt="palera1n logo" style="float: left;"/>
<h3 align="right">An iOS 15.0-16.3.1 work-in-progress, <br>(semi-)tethered checkm8 jailbreak</h3> 

<p  align="right" >
  <strong><a  href="https://ios.cfw.guide/installing-palera1n">Guide</a></strong>
  •
  <strong><a  href="COMMONISSUES.md">Common Issues</a></strong>
  •
  <strong><a  href="CHANGELOG.md">Changelog</a></strong>
  •
  <strong><a  href="https://dsc.gg/palera1n">Discord</a></strong>
  •
  <strong><a  href="https://twitter.com/palera1n">Twitter</a></strong>
</p>
<div class="clear"></div>

# How does it work?

It boots the device with multiple patches required. On first run, it'll boot a ramdisk which dumps your onboard blob, creates a fakefs (if using semi-tethered), installs the loader app, and patches your kernel. 

# Warnings
- We are **NOT** responsible for any data loss, or the result of a device being bricked. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device refuses to boot after using palera1n please use `iTunes` or `Finder` to restore iOS (or if you used tethered, use `--restorerootfs` at the end of your palera1n command).

- palera1n will never work in VirtualBox, VMware or any virtual machine that doesn't support a PCI passthrough.

# Requirements
- Needs a checkm8 vulnerable iOS device on iOS 15.x or 16.x (`A8` - `A11`)
	-	If you want the device to be semi-tethered, **you will need 5-10GB of space** for the fakefs. This means that 16GB devices cannot be semi-tethered
	- If you are on `A10(X)`, use [palera1n-c](https://github.com/palera1n/palera1n-c) instead for full SEP functionality (Passcode, TouchID, Apple Pay)
	- On `A11`, **you must disable your passcode while in the jailbroken state** (on iOS 16, you need to reset your device before proceeding with palera1n `A11`).
	- palera1n is currently broken on iOS 16.4

- **USB-A** cables are recommended to use, USB-C may have issues with palera1n and getting into DFU mode.

- A Linux or macOS computer
	- Python 3 must be installed.
	- AMD CPUs have an issue [with (likely) their USB controllers] that causes them to have a very low success rate with checkm8. It is not recommended that you use them with palera1n.
		- If your device does not successfully jailbreak, try a computer with an Intel or other CPU

# Still need help?

Join the [Support Discord](https://dsc.gg/palera1n) if you need help with palera1n :)

# Credits
<details><summary>palera1n Contributors</summary>
<p>

- [Nathan](https://github.com/verygenericname) for part of palera1n's development.
	- The ramdisk that dumps blobs, copies files, and duplicates rootfs is a slimmed down version of [SSHRD_Script](https://github.com/verygenericname/SSHRD_Script)
	- For modified [restored_external](https://github.com/verygenericname/sshrd_SSHRD_Script)
	- Also helped Mineek getting the kernel up and running and with the patches
	- Helping with adding multiple device support
	- Fixing issues relating to camera.. etc by switching to fsboot
	- [iBoot64Patcher fork](https://github.com/verygenericname/iBoot64Patcher)
- [Mineek](https://github.com/mineek) for more part of palera1n's development.
	- For the patching and booting commands
	- Adding tweak support
	- For patchfinders for RELEASE kernels
	- [Kernel15Patcher](https://github.com/mineek/PongoOS/tree/iOS15/checkra1n/Kernel15Patcher)
	- [Kernel64Patcher](https://github.com/mineek/Kernel64Patcher)
	- Work on jbinit, together with [Nick Chan](https://github.com/asdfugil)
- [Tom](https://github.com/plooshi) for a couple patches and bugfixes
	- For maintaining [Kernel64Patcher](https://github.com/palera1n/Kernel64Patcher)
- [Serena](https://github.com/SerenaKit) for helping with boot ramdisk.
- [Nick Chan](https://github.com/asdfugil) general help with patches and iBoot payload stuff
- [Dora](https://github.com/dora2-iOS) for iBoot payload and iBootpatcher2
- [alexia](https://github.com/0xallie) for the script to help get into DFU

</details>
<details><summary>Other credits for tools used in palera1n</summary>

- [Amy](https://github.com/elihwyma) for the [Pogo](https://github.com/elihwyma/Pogo) app
- [checkra1n](https://github.com/checkra1n) for the base of the kpf
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing [bootstrap](https://github.com/ProcursusTeam/Procursus)
- [m1sta](https://github.com/m1stadev) for [pyimg4](https://github.com/m1stadev/PyIMG4)
- [tihmstar](https://github.com/tihmstar) for [pzb](https://github.com/tihmstar/partialZipBrowser)/original [iBoot64Patcher](https://github.com/tihmstar/iBoot64Patcher)/original [liboffsetfinder64](https://github.com/tihmstar/liboffsetfinder64)/[img4tool](https://github.com/tihmstar/img4tool)
- [xerub](https://github.com/xerub) for [img4lib](https://github.com/xerub/img4lib) and [restored_external](https://github.com/xerub/sshrd) in the ramdisk
- [Cryptic](https://github.com/Cryptiiiic) for [iBoot64Patcher](https://github.com/Cryptiiiic/iBoot64Patcher) fork, and [liboffsetfinder64](https://github.com/Cryptiiiic/liboffsetfinder64) fork
- [libimobiledevice](https://github.com/libimobiledevice) for several tools used in this project (irecovery, ideviceenterrecovery etc), and [nikias](https://github.com/nikias) for keeping it up to date
- [Sam Bingner](https://github.com/sbingner) for [Substitute](https://github.com/sbingner/substitute)
</p>
</details>

<br>
<p align="center">
Thank you so much to our Patreons that make the future development possible! You may sub <a href="https://patreon.com/palera1n">here</a>, if you'd like to.</br>
</p>
<p align="center">
<a href="https://github.com/samh06"><img width=64 style="border-radius: 25%;" src="https://user-images.githubusercontent.com/18669106/206333607-881d7ca1-f3bf-4e18-b620-25de0c527315.png"></img></a>
<a href="https://havoc.app"><img width=64 style="border-radius: 25%;" src="https://docs.havoc.app/img/standard_icon.png"></img></a>
<a href="https://twitter.com/yyyyyy_public"><img width=64 style="border-radius: 25%;" src="https://cdn.discordapp.com/attachments/1054239098006683688/1072587455779328040/image.png?size=400"></img></a>
<a href="https://twitter.com/0xSp00kyb0t"><img width=64 style="border-radius: 25%;" src="https://pbs.twimg.com/profile_images/1603601553226620935/1t4yD1bD_400x400.jpg"></img></a>
<a href="https://chariz.com"><img width=64 src="https://chariz.com/img/favicon.png"></img></a>
<a href="https://twitter.com/stars6220"><img width=64 style="border-radius: 25%;" src="https://pbs.twimg.com/profile_images/1621062976982728706/pWVZQ-NO_400x400.jpg"></img></a>
<a href="https://github.com/beast9265"><img width=64 style="border-radius: 25%;" src="https://avatars.githubusercontent.com/u/79794946?v=4"></img></a>
<a href="https://twitter.com/0x7FF7"><img width=64 style="border-radius: 25%;" src="https://pbs.twimg.com/profile_images/1616888462665306113/AsjJvtyt_400x400.jpg">
<a href="https://github.com/TheFunnyMan16"><img width=64 style="border-radius: 25%;" src="https://cdn.discordapp.com/attachments/1050068822473842778/1082867264807772281/IMG_3942.jpg">
</img></a>
<a href="https://sideloadly.io/"><img width=64 style="border-radius: 25%;" src="https://sideloadly.io/icon.png"></img></a>
<a href="https://blog.stevesec.com/"><img width=64 style="border-radius: 25%;"  src="https://blog.stevesec.com/img/avatar.jpg"></img></a>
</p>
