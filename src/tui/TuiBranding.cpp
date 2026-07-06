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
