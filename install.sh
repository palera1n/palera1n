#!/usr/bin/env sh

set -eu

RED='\033[0;31m'
YELLOW='\033[0;33m'
DARK_GRAY='\033[90m'
LIGHT_CYAN='\033[0;96m'
DARK_CYAN='\033[0;36m'
NO_COLOR='\033[0m'
BOLD='\033[1m'

printf '%b' "\033c"
printf '%s\n' '::'
printf '%s\n' ':: =====================>'
printf '%s\n' ':: palera1n install script'
printf '%s\n' ':: =====================>'
printf '%s\n' ''

# =========
# Logging
# =========

error() {
    printf '%b\n' " - [${DARK_GRAY}$(date +'%m/%d/%y %H:%M:%S')${NO_COLOR}] ${RED}${BOLD}<Error>${NO_COLOR}: ${RED}$1${NO_COLOR}" >&2
}

info() {
    printf '%b\n' " - [${DARK_GRAY}$(date +'%m/%d/%y %H:%M:%S')${NO_COLOR}] ${DARK_CYAN}${BOLD}<Info>${NO_COLOR}: ${DARK_CYAN}$1${NO_COLOR}"
}

warning() {
    printf '%b\n' " - [${DARK_GRAY}$(date +'%m/%d/%y %H:%M:%S')${NO_COLOR}] ${YELLOW}${BOLD}<Warning>${NO_COLOR}: ${YELLOW}$1${NO_COLOR}"
}

if [ "$(id -u)" -eq 0 ]; then
    error "Run this script without sudo!"
    exit 1
fi

# =========
# Variables
# =========

os="$(uname)"
os_name="$os"
prefix_path="${XDG_BIN_HOME:-$HOME/.local}"
bin_path="$prefix_path/bin"
install_path="$bin_path/palera1n"
old_install_path="/usr/local/bin/palera1n"

download() {
    local url="$1"
    local dest="${2:-$install_path}"

    status=$(curl --progress-bar --write-out '%{http_code}' -Lo "$dest" "$url")

    if [ "$status" -ne 200 ]; then
        error "palera1n failed to download. Check your internet connection. (HTTP Status: $status)"
        rm -f -- "$dest"
        exit 1
    fi

    chmod +x "$dest"
}

remove_palera1n() {
    local removed=false

    if [ -e "${install_path}" ]; then
        rm -f -- "${install_path}"
        rm -f -- "${prefix_path}/share/man/man1/palera1n.1" 2>/dev/null || true
        rm -f -- "${prefix_path}/share/man/man8/p1ctl.8" 2>/dev/null || true
        removed=true
    fi

    if [ -e "${old_install_path}" ]; then
        info "Removing legacy installation from ${old_install_path} (sudo required)..."
        sudo rm -f -- "${old_install_path}"
        removed=true
    fi

    if [ "${removed}" = "true" ]; then
        info "Successfully removed."
    else
        warning "palera1n installation was not found."
    fi
}

add_to_path() {
    local path_dir="$1"
    local export_line="export PATH=\"\$PATH:${path_dir}\""

    case ":$PATH:" in
        *":${path_dir}:"*)
            return 0
            ;;
    esac

    local target_profile=""
    case "${SHELL:-}" in
        */zsh)  target_profile="$HOME/.zshrc" ;;
        */bash) target_profile="$HOME/.bashrc" ;;
        *)      target_profile="$HOME/.profile" ;;
    esac

    if [ -f "$target_profile" ] && grep -Fqx "$export_line" "$target_profile" 2>/dev/null; then
        export PATH="$PATH:$path_dir"
        return 0
    fi

    printf 'Add "%s" to your PATH permanently? [Y/n] ' "$path_dir"
    read -r reply

    case "$reply" in
        ""|[Yy]|[Yy][Ee][Ss])
            mkdir -p "$(dirname "$target_profile")"
            printf '\n# Added by palera1n installer\n%s\n' "$export_line" >> "$target_profile"
            export PATH="$PATH:$path_dir"
            info "Added ${path_dir} to PATH in ${target_profile}."
            ;;
        *)
            info "Skipping..."
            ;;
    esac
}

print_help() {
    cat << EOF
Usage: $0 [-h|--help] [-r|--remove]

Options:
    -h, --help          Print this help message
    -r, --remove        Uninstall palera1n
EOF
}

# =========
# Dependencies
# =========

case "$os" in
    Linux)
        if ! command -v curl >/dev/null 2>&1; then
            error "curl is required to run this script."
            exit 1
        fi
        if ! command -v tar >/dev/null 2>&1; then
            error "tar is required to run this script."
            exit 1
        fi
    ;;
esac

# =========
# OS and Architecture
# =========

case "$os" in
    Linux)
        arch_check=$(uname -m)
        os_name="Linux"
    ;;
    Darwin)
        os_version=$(uname -r | cut -d. -f1)
        if [ "$os_version" -gt 15 ]; then
            os_name="macOS"
        elif [ "$(uname -m | head -c2)" = "iP" ]; then
            error "The palera1n installer script is not meant for iOS devices directly. Please run on a PC."
            exit 1
        else
            os_name="Mac OS X"
        fi
        arch_check=$(uname -m)
    ;;
    *)
        error "Unknown or unsupported OS ($os)."
        exit 1
    ;;
esac

if [ "$os" = "Linux" ]; then
    if grep -qi Microsoft /proc/version 2>/dev/null; then
        error "palera1n is not supported on WSL. Please use another supported platform."
        exit 1
    fi
fi

case "$arch_check" in
    x86_64* | amd64)
        arch="x86_64"
    ;;
    i?86 | x86*)
        arch="x86"
    ;;
    aarch64* | arm64*)
        arch="arm64"
    ;;
    arm*)
        arch="armel"
    ;;
    *)
        error "Unknown or unsupported architecture ($arch_check)."
        exit 1
    ;;
esac

# =========
# Args
# =========

case "${1:-}" in
    "") ;;
    "-r" | "--remove")
        remove_palera1n
        exit 0
        ;;
    "-h" | "--help")
        print_help
        exit 0
        ;;
    *)
        error "Invalid option: \"$1\""
        print_help
        exit 1
        ;;
esac

# =========
# Run
# =========

download_version=$(curl -s https://api.github.com/repos/palera1n/palera1n/releases/latest | grep -o '"tag_name": "[^"]*' | sed 's/"tag_name": "//' || true)

if [ -z "$download_version" ]; then
    error "Could not retrieve the latest release version from GitHub API."
    exit 1
fi

info "Detected environment: $os_name ($arch)"
info "Targeting release tag: ${download_version}"

download_prefix="https://github.com/palera1n/palera1n/releases/download"
mkdir -p "$bin_path"

if [ -f "$install_path" ] || [ -f "$old_install_path" ]; then
    warning "Existing palera1n installation found. Replacing..."
    remove_palera1n
fi

case "$os" in
    Linux)
        tgz_url="${download_prefix}/${download_version}/palera1n-linux-${arch}.tar.gz"
        bin_url="${download_prefix}/${download_version}/palera1n-linux-${arch}"

        if curl -fsLI "$tgz_url" >/dev/null 2>&1; then
            info "Installing from Linux tarball..."
            curl -fsSL "$tgz_url" | tar -xz -C "$prefix_path"
        else
            info "Installing standalone Linux binary..."
            download "$bin_url"
            chmod +x "$install_path" 2>/dev/null || true
        fi
        ;;
    Darwin)
        dmg_url="${download_prefix}/${download_version}/palera1n-macos-universal.dmg"
        tgz_url="${download_prefix}/${download_version}/palera1n-macos-${arch}.tar.gz"
        bin_url="${download_prefix}/${download_version}/palera1n-macos-${arch}"

        if curl -fsLI "$dmg_url" >/dev/null 2>&1; then
            error "DMG releases are not supported by this CLI installer. Please download the DMG manually."
            exit 1
        fi

        if curl -fsLI "$tgz_url" >/dev/null 2>&1; then
            info "Installing from macOS tarball..."
            curl -fsSL "$tgz_url" | tar -xz -C "$prefix_path"
        else
            info "Installing standalone macOS binary..."
            download "$bin_url"
            chmod +x "$install_path" 2>/dev/null || true
        fi
        ;;
esac

add_to_path "$bin_path"

if [ -f "$install_path" ]; then
    if ! "$install_path" --version > /dev/null 2>&1; then
        error "palera1n installation is corrupted. Please check your internet connection and try again."
        exit 1
    fi

    info "palera1n installed successfully at ${install_path}."
    info "Type \`palera1n\` in your terminal to get started!"
    exec "${SHELL:-/bin/sh}" -l
else 
    error "palera1n failed to install. Please check your internet connection and try again."
    exit 1
fi
