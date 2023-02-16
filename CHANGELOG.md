# Changelog

## 1.4.2
- Add iOS 16.4 support
- Various fixes

## 1.4.1
- ibot.patched fix
- Use payload on the X
- Other random changes/fixes

## 1.4.0

- Does not mount user data partition for iPhone X compatibility, isn't even really needed anymore
- Deploys files to the rootfs (fakefs if needed)
- Fix deviceid finding
- Use apticket.der because dumping rdisk seems to freeze
- Add /.installed_palera1n with info
- uicache loader app on boot (no more Tips app hijacking)
- Fix rootless
- Webkit fix on 16
- Switch to local boot
- Fix home button on iPhone 7(+) and 8(+)
- Increase stability
- Supports 15.0-16.2 on all checkm8 devices

## 1.3.0

- Fully fix deep sleep bug
- Supports 15.0-15.7.1 on all checkm8 devices
- Increase stability
- Fix TrollStore, camera, and screen recording

## 1.2.0

- Numerous fixes
- Let Pogo install tweak support

## 1.1.1

- Support iPad beta URLs
- Make sure auto-boot is always set to false (unless restoring rootfs)
- Wait for sshd to start before running postboot
- Fix Tips check error
- Fix --restorerootfs

## 1.1.0

- Check if Tips is installed
- Only prompt for disclaimer once
- Check for DFU
