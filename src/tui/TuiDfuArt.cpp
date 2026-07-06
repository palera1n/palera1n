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
