# Changelog

## 1.4.0

- Add 16.0-16.1.1 support
- Uicache loader app on boot (no more Tips app hijacking)
- Fix deviceid finding
- Use apticket.der over dumping rdisk1
- Add info.json for palera1n-info tweak
- Fix rootless
- Semi-tethered WebKit fix on iOS 16
- Switch to local boot over fsboot

## 1.3.0

- Fully fix deep sleep bug
- Support 15.0-15.7.1 on all checkm8 devices
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
