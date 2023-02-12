<img align="left" height="120" src="https://cdn.discordapp.com/attachments/1017854329887129611/1073858292159352862/thing.png" alt="palera1n logo" style="float: left;"/>
<h3 align="right">An iOS 15.0-16.3 work-in-progress, <br>(semi-)tethered checkm8 jailbreak</h3> 

<p  align="right" >
  <strong><a  href="https://cdn.nickchan.lol/palera1n/c-rewrite/releases/v2.0.0-beta.1/palera1n.1.html">Usage</a></strong>
  •
  <strong><a  href="https://github.com/palera1n/palera1n-c/graphs/contributors">Contributors</a></strong>
  •
  <strong><a  href="https://dsc.gg/palera1n">Discord</a></strong>
  •
  <strong><a  href="https://twitter.com/palera1n">Twitter</a></strong>
</p>
<div class="clear"></div>
<br></br>

### You can obtain v2.0.0 beta [here](https://github.com/palera1n/palera1n-c/releases).

# Warnings
- This palera1n rewrite has noticeably different arguments compared to older palera1n, would recommend you take a look at how to use palera1n-c [here](https://cdn.nickchan.lol/palera1n/c-rewrite/releases/v2.0.0-beta.1/palera1n.1.html).

- This does **NOT** support tethered creation or booting. You must use older [palera1n](https://github.com/palera1n/palera1n).

- We are **NOT** responsible for any data loss, or the result of a device being bricked. The user of this program accepts responsibility should something happen to their device. While nothing should happen, jailbreaking has risks in itself. If your device is stuck in recovery, please run futurerestore `--exit-recovery`, or use `irecovery -n`.

- palera1n-c will never work in VirtualBox, VMware or any virtual machine that doesn't support a PCI passthrough.

# Requirements
- Needs a checkm8 vulnerable iOS device on iOS 15.x or 16.x (`A8` - `A11`)
	-	If using Rootful, **you will need 5-10GB of space** for the fakefs. This means that 16GB devices cannot be semi-tethered with Rootful.
	- On `A11`, **you must disable your passcode while in the jailbroken state** (on iOS 16, you need to reset your device before proceeding with palera1n `A11`).

- **USB-A** cables are recommended to use, USB-C may have issues with palera1n and getting into DFU mode.

- A Linux or macOS computer
	- AMD CPUs (not AMD Mobile) have an issue [with (likely) their USB controllers] that causes them to have a very low success rate with checkm8. It is not recommended that you use them with palera1n.
		- If your device does not successfully jailbreak, try a computer with an Intel or other CPU

# Reporting Issues

Make sure you provide full details on your device, such as:
- iDevice
- iOS Version
- Passcode enabled?
- Logs, if panicked send latest panic-full from your iDevice.

Using `-V` and `-v` would help with debugging.

Join the [Support Discord](https://dsc.gg/palera1n), and find the palera1n-c thread under #jailbreak [here](https://discord.com/channels/1028398973452570725/1074251283143344180).

# Credits
<details><summary>palera1n-c Contributors and Credits</summary>
<p>

- [Nebula](https://github.com/itsnebulalol), palera1n owner and Python rewrite lead developer
- [Mineek](https://github.com/mineek)
- [Tom](https://github.com/guacaplushy) for updated ploosh kpf
- [Lakhan Lothiyi](https://github.com/llsc12) for palera1n loader app
- [checkra1n](https://github.com/checkra1n) for the base of the kpf
- [the Procursus Team](https://github.com/ProcursusTeam) for the amazing [bootstrap](https://github.com/ProcursusTeam/Procursus)
- [Évelyne](https://github.com/evelyneee) for [ElleKit](https://github.com/evelyneee/ellekit), rootless tweak injection
- [Sam Bingner](https://github.com/sbingner) for [Substitute](https://github.com/sbingner/substitute), rootful tweak injection

</details>
</p>

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
<a href="https://twitter.com/0x7FF7"><img width=64 style="border-radius: 25%;" src="https://pbs.twimg.com/profile_images/1616888462665306113/AsjJvtyt_400x400.jpg"></img></a>
<a href="https://sideloadly.io/"><img width=64 style="border-radius: 25%;" src="https://sideloadly.io/icon.png"></img></a>
<a href="https://blog.stevesec.com/"><img width=64 style="border-radius: 25%;"  src="https://blog.stevesec.com/img/avatar.jpg"></img></a>
</p>
