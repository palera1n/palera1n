#!/usr/bin/env bash

set -e

cat > /etc/udev/rules.d/turdusra1n.rules <<'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="1227", GROUP="plugdev", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="1338", GROUP="plugdev", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="4141", GROUP="plugdev", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="1281", GROUP="plugdev", MODE="0666"
EOF

getent group plugdev >/dev/null 2>&1 || groupadd plugdev
usermod -aG plugdev "$USER"

udevadm control --reload-rules
udevadm trigger
