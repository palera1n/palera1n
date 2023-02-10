curl -LOOOOOO \
            https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v3.3.0.tar.gz \
            https://github.com/libusb/libusb/releases/download/1.0.26/libusb-1.0.26.tar.bz2 \
            https://mirror-hk.koddos.net/gnu/readline/readline-8.2.tar.gz \
            https://www.jedsoft.org/releases/slang/slang-2.3.3.tar.bz2 \
            https://releases.pagure.org/newt/newt-0.52.23.tar.gz \
            https://github.com/rpm-software-management/popt/archive/master-release.tar.gz

tar xf master.tar.gz
tar xf libusb-1.0.26.tar.bz2
tar xf readline-8.2.tar.gz
tar xf slang-2.3.3.tar.bz2
tar xf newt-0.52.23.tar.gz
tar xf master-release.tar.gz

git clone --depth=1 https://github.com/libimobiledevice/libplist
git clone --depth=1 https://github.com/libimobiledevice/libimobiledevice-glue
git clone --depth=1 https://github.com/libimobiledevice/libirecovery
git clone --depth=1 https://github.com/libimobiledevice/libusbmuxd
git clone --depth=1 https://github.com/libimobiledevice/libimobiledevice