/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#if defined(__linux__)

#include "udev_helper.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> // waitpid
#include <unistd.h>

static const char *current_username(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_name && pw->pw_name[0] != '\0') {
        return pw->pw_name;
    }
    const char *env_user = getenv("USER");
    return (env_user && env_user[0] != '\0') ? env_user : NULL;
}

int udev_rules_exist(void)
{
    return access("/etc/udev/rules.d/turdusra1n.rules", F_OK) == 0;
}

int add_udev_rules(void)
{
    static const char *script =
        "set -e\n"
        "cat > /etc/udev/rules.d/turdusra1n.rules <<'EOF'\n"
        "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"05ac\", ATTRS{idProduct}==\"1227\", GROUP=\"plugdev\", MODE=\"0666\"\n"
        "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"05ac\", ATTRS{idProduct}==\"1338\", GROUP=\"plugdev\", MODE=\"0666\"\n"
        "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"05ac\", ATTRS{idProduct}==\"4141\", GROUP=\"plugdev\", MODE=\"0666\"\n"
        "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"05ac\", ATTRS{idProduct}==\"1281\", GROUP=\"plugdev\", MODE=\"0666\"\n"
        "EOF\n"
        "getent group plugdev >/dev/null 2>&1 || groupadd plugdev\n"
        "if [ -n \"$1\" ]; then\n"
        "    usermod -aG plugdev \"$1\"\n"
        "fi\n"
        "udevadm control --reload-rules\n"
        "udevadm trigger\n";

    const char *user = current_username();

    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    }

    if (pid == 0) {
        execlp(
            "pkexec",
            "pkexec",
            "sh",
            "-c",
            script,
            "turdusra1n-udev",
            user ? user : "",
            (char *)NULL
        );
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        return 1;
    }

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

#endif // defined(__linux__)
