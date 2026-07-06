#ifdef WITH_TUI

#ifndef TUI_DFU_ART_HPP
#define TUI_DFU_ART_HPP

#include <string>

struct DfuAsciiPreview {
    const char* imageName;
    const char* const* lines;
    int lineCount;
    int xOffset;
    int yOffset;
};

const DfuAsciiPreview* find_dfu_ascii_preview(const std::string& imageName);
void draw_dfu_ascii_preview(const DfuAsciiPreview& preview, int sx, int sy);

#endif // TUI_DFU_ART_HPP

#endif // WITH_TUI
