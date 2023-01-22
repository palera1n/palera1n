<h1 align="center">
  palera1n
</h1>

<p align="center">
  <a href="#">
    <img src="https://img.shields.io/badge/made%20with-love-E760A4.svg" alt="Made with love">
  </a>
  <a href="https://github.com/palera1n/palera1n-c/blob/main/LICENSE" target="_blank">
    <img src="https://img.shields.io/github/license/palera1n/palera1n-c.svg" alt="License">
  </a>
  <a href="https://github.com/palera1n/palera1n-c/graphs/contributors" target="_blank">
    <img src="https://img.shields.io/github/contributors/palera1n/palera1n-c.svg" alt="Contributors">
  </a>
  <a href="https://github.com/palera1n/palera1n-c/commits/main" target="_blank">
    <img src="https://img.shields.io/github/commit-activity/w/palera1n/palera1n-c.svg" alt="Commits">
  </a>
  <a href="https://dsc.gg/palera1n" target="_blank">
    <img src="https://img.shields.io/discord/1028398973452570725?label=discord" alt="Discord">
  </a>
</p>

<p align="center">
iOS 15.0-16.3 work in progress, semi-tethered checkm8 jailbreak
</p>

# BEFORE YOU USE

This is mainly for ROOTLESS, and is NOT READY. It was made public so you can contribute and explore the code. This is not the final product.

This version of palera1n supports booting rootful setups with the `-f <root device>` option (like `-f disk0s1s8`), but not create them.

Loader app does appear on rootless but cannot bootstrap, you may use the SSH server listening at 127.0.0.1:44 (IPv4) or ::1:44 (IPv6).

The override overlay option does not currently work and will upload a corrupted overlay.

If the device is previously jailbroken, the jailbreak environment is automatically prepared and ends with a respring. 
You can use the `-s` option to skip this behaviour.

Usbmuxd is only required for starting in normal mode, it is not required when started from recovery or DFU mode. (Usbmuxd is preinstalled with macOS)

The `--force-revert` option only works on rootless.

You can obtain the latest build of palera1n-c [here](https://cdn.nickchan.lol/palera1n/artifacts/c-rewrite/).

# This is a work in progress.

Read this throughly, feel free to ask questions, know the risks. If you want to ask questions, either:

1. Ask in the [palera1n Discord](https://discord.gg/4S3yUMxuQH)
2. Ask in the r/jailbreak Discord #palera1n channel

Please, please, please, provide necessary info:

- iOS version and device (eg. iPhone 7+ 15.1, iPhone 6s 15.3.1)
- Computer's OS and version (eg. Ubuntu 22.04, macOS 13.0)
- The command you ran

**DO NOT** harass tweak devs if tweaks don't work. Refer to [here](https://github.com/itsnebulalol/ios15-tweaks) for compatiblity.

# Patreons

Thank you so much to our Patreons that make the future development possible! You may sub [here](https://patreon.com/palera1n), if you'd like to. If you subscribe, please message [Nebula](https://twitter.com/itsnebulalol) in any way preferred to have you put here.

<a href="https://github.com/samh06"><img width=64 src="https://user-images.githubusercontent.com/18669106/206333607-881d7ca1-f3bf-4e18-b620-25de0c527315.png"></img></a>
<a href="https://havoc.app"><img width=64 src="https://docs.havoc.app/img/standard_icon.png"></img></a>
<a href="https://twitter.com/yyyyyy_public"><img width=64 src="https://pbs.twimg.com/profile_images/1429332550112079876/dQQgsURc_400x400.jpg"></img></a>
<a href="https://twitter.com/0xSp00kyb0t"><img width=64 src="https://pbs.twimg.com/profile_images/1603601553226620935/1t4yD1bD_400x400.jpg"></img></a>
<a href="https://chariz.com"><img width=64 src="https://chariz.com/img/favicon.png"></img></a>
<a href="https://twitter.com/stars6220"><img width=64 src="https://pbs.twimg.com/profile_images/1606990218925670400/Y4JBl6OS_400x400.jpg"></img></a>
<a href="https://github.com/beast9265"><img width=64 src="https://avatars.githubusercontent.com/u/79794946?v=4"></img></a>

# What does this do?

It boots the device with patches for the jailbreak. 

**WARNING**: I am NOT responsible for any data loss. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device is stuck in recovery, please run `futurerestore --exit-recovery`, or use `irecovery -n`.

On A11, you **must disable your passcode while in the jailbroken state**. We don't have an A11 SEP exploit yet.

<!--# Usage
See the [wiki](https://github.com/palera1n/palera1n-c/wiki).-->

# Credits

- [Nebula](https://github.com/itsnebulalol), palera1n owner and Python rewrite lead developer
- [Nathan](https://github.com/verygenericname)
- [Mineek](https://github.com/mineek)
    - Work on jbinit, together with [Nick Chan](https://github.com/asdfugil)
- [Amy](https://github.com/elihwyma) for the [Pogo](https://github.com/elihwyma/Pogo) app
- [checkra1n](https://github.com/checkra1n) for the base of the kpf
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing [bootstrap](https://github.com/ProcursusTeam/Procursus)
- [F121](https://github.com/F121Live) for helping test
- [Tom](https://github.com/guacaplushy) for a couple patches and bugfixes
- [Nick Chan](https://github.com/asdfugil) general help with patches and jbinit
- [Serena](https://github.com/SerenaKit) for helping with boot ramdisk
- [Évelyne](https://github.com/evelyneee) for ElleKit, rootless tweak injection
