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

#ifndef TUI_TEXT_HPP
#define TUI_TEXT_HPP

#include <ncurses.h>

#include <string>
#include <vector>

namespace tui_text {

inline void append_wrapped_lines(const std::string& text, int width, std::vector<std::string>& out) {
    if (width <= 0) {
        return;
    }

    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }

        std::string paragraph = text.substr(start, end - start);
        size_t cursor = 0;

        if (paragraph.empty()) {
            out.push_back("");
        }

        while (cursor < paragraph.size()) {
            size_t remaining = paragraph.size() - cursor;
            if (remaining <= static_cast<size_t>(width)) {
                out.push_back(paragraph.substr(cursor));
                break;
            }

            size_t cut = cursor + static_cast<size_t>(width);
            size_t split = paragraph.rfind(' ', cut);
            if (split == std::string::npos || split <= cursor) {
                split = cut;
            }

            out.push_back(paragraph.substr(cursor, split - cursor));
            cursor = split;
            while (cursor < paragraph.size() && paragraph[cursor] == ' ') {
                ++cursor;
            }
        }

        if (end == text.size()) {
            break;
        }
        start = end + 1;
    }
}

inline int draw_wrapped_text(int y, int x, int width, const std::string& text, int max_lines = 0) {
    if (width <= 0 || text.empty()) {
        return 0;
    }

    std::vector<std::string> lines;
    append_wrapped_lines(text, width, lines);

    int rendered = 0;
    for (const auto& line : lines) {
        if (max_lines > 0 && rendered >= max_lines) {
            break;
        }
        mvprintw(y + rendered, x, "%.*s", width, line.c_str());
        ++rendered;
    }

    return rendered;
}

} // namespace tui_text

#endif // TUI_TEXT_HPP

#endif // WITH_TUI
