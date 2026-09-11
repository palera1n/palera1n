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

#include "udevhelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // waitpid
#include <sys/stat.h> // chmod

#include "gen/embedded/add_udev_rules.h"

int udev_rules_exist(void)
{
    return access("/etc/udev/rules.d/turdusra1n.rules", F_OK) == 0;
}

int add_udev_rules(void)
{
    FILE *f = fopen("/tmp/udevhelper.sh", "wb");
    if (!f)
        return 1;

    fwrite(
        embedded_add_udev_rules_sh,
        1,
        embedded_add_udev_rules_sh_len,
        f
    );
    fclose(f);

    chmod("/tmp/udevhelper.sh", 0700);

    pid_t pid = fork();

    if (pid < 0) {
        unlink("/tmp/udevhelper.sh");
        return 1;
    }

    if (pid == 0) {
        execlp("pkexec", "pkexec", "/tmp/udevhelper.sh", (char *)NULL);
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    unlink("/tmp/udevhelper.sh");

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

#endif // defined(__linux__)
