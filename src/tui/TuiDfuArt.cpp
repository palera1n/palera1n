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

#ifdef WITH_TUI

#include "TuiDfuArt.hpp"

#include <ncurses.h>

static const char* kIpadArt[] = {
    "                   __    ",
    "  ┌───────────────────┐  ",
    "  │       o ===       │  ",
    "  │███████████████████│  ",
    " |│███████████████████│  ",
    " |│███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │███████████████████│  ",
    "  │        ┌─┐        │  ",
    "  │        └─┘        │  ",
    "  └───────────────────┘  ",
};

static const char* kIphoneSeArt[] = {
    "             __    ",
    "  ┌─────────────┐  ",
    "  │    o ===    │  ",
    "  │█████████████│  ",
    " |│█████████████│  ",
    " |│█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │     ┌─┐     │  ",
    "  │     └─┘     │  ",
    "  └─────────────┘  ",
};

static const char* kIphone7Art[] = {
    "                   ",
    "  ┌─────────────┐  ",
    "  │    o ===    │  ",
    "  │█████████████│  ",
    " |│█████████████│| ",
    " |│█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │     ┌─┐     │  ",
    "  │     └─┘     │  ",
    "  └─────────────┘  ",
};

static const char* kIphoneXArt[] = {
    "                   ",
    "  ┌─────────────┐  ",
    "  │███ o === ███│  ",
    "  │█████████████│  ",
    " |│█████████████││ ",
    " |│█████████████││ ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  │█████████████│  ",
    "  └─────────────┘  ",
};

static const char* kRemoteArt[] = {
    "               ",
    "  ┌─────────┐  ",
    "  │         │  ",
    "  │         │  ",
    "  │         │  ",
    "  │ ┌─┐ ┌─┐ │  ",
    "  │ └─┘ └─┘ │  ",
    "  │ ┌─┐ ┌─┐ │  ",
    "  │ └─┘ │+│ │  ",
    "  │ ┌─┐ │-│ │  ",
    "  │ └─┘ └─┘ │  ",
    "  │         │  ",
    "  │         │  ",
    "  │         │  ",
    "  └─────────┘  ",
};

static const char* kAtvStdBrdArt[] = {
    " art not yet implemented ",
};

static const DfuAsciiPreview kPreviews[] = {
    { "ipad", kIpadArt, 16, 20, -1 },
    { "ipadmini", kIpadArt, 16, 19, -1 },
    { "ipodtouch", kIphoneSeArt, 16, 22, -1 },
    { "ipodtouch7", kIpadArt, 16, 20, -1 },
    { "siriremote", kRemoteArt, 15, 22, -1 },
    { "atv_std_brd", kAtvStdBrdArt, 1, 20, -1 },
    { "iphonese", kIphoneSeArt, 16, 22, -1 },
    { "iphone6s", kIphone7Art, 16, 22, -1 },
    { "iphone7", kIphone7Art, 16, 22, -1 },
    { "iphone8", kIphone7Art, 16, 22, -1 },
    { "iphonex", kIphoneXArt, 16, 22, -1 },
};

const DfuAsciiPreview* find_dfu_ascii_preview(const std::string& imageName) {
    for (const auto& preview : kPreviews) {
        if (imageName == preview.imageName) {
            return &preview;
        }
    }
    return nullptr;
}

void draw_dfu_ascii_preview(const DfuAsciiPreview& preview, int sx, int sy) {
    const int start_x = sx + preview.xOffset;
    const int start_y = sy + 4 + preview.yOffset;

    attron(A_DIM);
    for (int i = 0; i < preview.lineCount; ++i) {
        mvprintw(start_y + i, start_x, "%s", preview.lines[i]);
    }
    attroff(A_DIM);
}

#endif // WITH_TUI
