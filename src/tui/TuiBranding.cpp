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

#include "TuiBranding.hpp"

#include <algorithm>
#include <cstring>

#include <ncurses.h>

static constexpr int kBrandingLeftTrim = 3;

void draw_tui_sidebar_branding(int y, int x, int width) {
    static const char* art[] = {
        "       ++       ",
        "      #@@#      ",
        "     +@@@@+     ",
        "    :@@@@@@:    ",
        "    #@@@@@@#    ",
        "   +@@@@@@@@+   ",
        "  :#*@@@@@@*#:  ",
        "  %##palera1n% ",
        " .####%@@%####. ",
        " @############@ ",
        "=@@*########*@@=",
        "+@@@@%####%@@@@+",
        ":@@@@%%**%%@@@@:",
        " +@@%%*@@*%%@@+ ",
        "   ++%@@@@%++   ",
        "      :++:      "
    };
    static const int art_count = static_cast<int>(sizeof(art) / sizeof(art[0]));

    int max_art_width = 0;
    for (int i = 0; i < art_count; ++i) {
        const int line_width = std::max(0, static_cast<int>(std::strlen(art[i])) - kBrandingLeftTrim);
        max_art_width = std::max(max_art_width, line_width);
    }

    const int centered_x = x + std::max(0, (width - max_art_width) / 2);
    const int art_x = std::max(x, centered_x - 7);

    attron(A_DIM);
    for (int i = 0; i < art_count; ++i) {
        const char* line = art[i];
        const int line_len = static_cast<int>(std::strlen(line));
        const int trim = std::min(kBrandingLeftTrim, line_len);
        mvprintw(y + i, art_x, "%.*s", width, line + trim);
    }
    attroff(A_DIM);
}

#endif // WITH_TUI
