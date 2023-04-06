#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <hello_imgui/hello_imgui.h>
#include <imgui/imgui_internal.h>

#include <common.h>
#include <design.h>


/* -----------------  [  Animation Varibles ]  ----------------- */

float btn_w = 0.0f;
float btn_h = 0.0f;
float btn_x = 0.0f;
float btn_y = 0.0f;
float box_w = 0.0f;
float box_h = 0.0f;
float box_x = 0.0f;
float box_y = 0.0f;


/* -----------------  [  UI Init/Reset  ]  ----------------- */

void init_colors() {
    auto &colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.21f, 0.23f, 0.31f, 1.0f};
    colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Border
    colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.0f};
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Text
    colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

    // Headers
    colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};

    // Popups
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13f, 0.15f, 0.7f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.13f, 0.13f, 0.15f, 0.7f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.13f, 0.13f, 0.15f, 0.7f};

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

    // Seperator
    colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
    colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
    colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
}

void init_style() {
    auto &style = ImGui::GetStyle();
    style.TabRounding = 0;
    style.ScrollbarRounding = 0;
    style.WindowRounding = 0;
    style.GrabRounding = 0;
    style.FrameRounding = 20;
    style.PopupRounding = 0;
    style.ChildRounding = 0;
}

void reset_design() {
    ImGui::SetCursorPos(ImVec2(0, 0));
    init_colors();
    init_style();
}

void reset_views() {
    waiting_for_device = false;
    unsupported_device = false;
    supported_device = false;
    recovery_wait = false;
    jailbreak = false;
    enter_dfu = false;
    dfu_helper = false;
}



/* -----------------  [  UI Element Styles  ]  ----------------- */

void image_btn_style() {
    auto &colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Button] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.95f, 0.95f, 0.95f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.95f, 0.95f, 0.95f, 1.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.95f, 0.95f, 0.95f, 1.0f};
    colors[ImGuiCol_Text] = ImVec4{0.1f, 0.1f, 0.1f, 1.0f};
}

void image_btn_style_alt() {
    auto &colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Button] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_Text] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
}



/* -----------------  [  IDK Anymore  ]  ----------------- */

std::string addstr2(const char *str1, const char *str2) {
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    int buf_size = (int)str1_len + (int)str2_len + 1;
    char buffer[buf_size];
    sprintf(buffer, str1, str2);
    return buffer;
}

std::string addstr3(const char *str1, const char *str2, const char *str3) {
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    size_t str3_len = strlen(str3);
    int buf_size = (int)str1_len + (int)str2_len + (int)str3_len + 1;
    char buffer[buf_size];
    sprintf(buffer, str1, str2, str3);
    return buffer;
}

std::string addstr4(const char *str1, const char *str2, const char *str3, const char *str4) {
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    size_t str3_len = strlen(str3);
    size_t str4_len = strlen(str4);
    int buf_size = (int)str1_len + (int)str2_len + (int)str3_len + (int)str4_len + 1;
    char buffer[buf_size];
    sprintf(buffer, str1, str2, str3, str4);
    return buffer;
}



/* -----------------  [  UI Colors  ]  ----------------- */

ImVec4 hex_color(int hex, float alpha) {
    float r = ((hex >> 16) & 0xFF) / 255.0f;
    float g = ((hex >> 8) & 0xFF) / 255.0f;
    float b = ((hex) & 0xFF) / 255.0f;

    return ImVec4{r, g, b, alpha};
}

ImColor hex_imcolor(int hex, float alpha) {
    float r = ((hex >> 16) & 0xFF) / 255.0f;
    float g = ((hex >> 8) & 0xFF) / 255.0f;
    float b = ((hex) & 0xFF) / 255.0f;

    return ImColor{r, g, b, alpha};
}



/* -----------------  [  UI Element States  ]  ----------------- */

bool is_hovered(const char* id, int w, int h) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiID btn_id = window->GetID(id);
    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.x + h));
    bool hovered = false;
    ImGui::ButtonBehavior(bb,btn_id,&hovered,NULL);
    return hovered;
}

bool is_pressed(const char* id, int w, int h) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiID btn_id = window->GetID(id);
    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.x + h));
    bool pressed = false;
    ImGui::ButtonBehavior(bb,btn_id,NULL,&pressed);
    return pressed;
}



/* -----------------  [  Common UI Types  ]  ----------------- */

void label(const char *text, ImVec2 location, ImVec4 color, int font) {
    auto &style = ImGui::GetStyle();
    ImGuiIO &io = ImGui::GetIO();

    ImGui::SetCursorPos(location);
    ImGui::PushFont(io.Fonts->Fonts[font]);

    auto &colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = color;
    

    ImGui::Text(text);
    ImGui::PopFont();
    reset_design();
}

void draw_box(ImVec2 location, ImVec2 size, ImVec4 color, float radius) {
    auto &colors = ImGui::GetStyle().Colors;
    auto &style = ImGui::GetStyle();

    float temp_rounding = style.FrameRounding;
    ImVec4 temp_color_idle = colors[ImGuiCol_FrameBg];
    ImVec4 temp_color_hover = colors[ImGuiCol_FrameBgHovered];
    ImVec4 temp_color_active = colors[ImGuiCol_FrameBgActive];

    style.FrameRounding = radius;
    colors[ImGuiCol_FrameBg] = color;
    colors[ImGuiCol_FrameBgHovered] = color;
    colors[ImGuiCol_FrameBgActive] = color;

    ImGui::SetCursorPos(location);
    ImGui::ProgressBar(0.0f, size, "");

    style.FrameRounding = temp_rounding;
    colors[ImGuiCol_FrameBg] = temp_color_idle;
    colors[ImGuiCol_FrameBgHovered] = temp_color_hover;
    colors[ImGuiCol_FrameBgActive] = temp_color_active;
}



/* -----------------  [  Animation Systems  ]  ----------------- */

void anim_btn(const char* id, ImVec2 size, ImVec2 size_to, ImVec2 pos, ImVec2 pos_to) {
    ImGuiIO &io = ImGui::GetIO();
    float global = 25.0f;
    bool new_w_positive,new_h_positive,new_x_positive,new_y_positive = false;
    float new_w,new_h,new_x,new_y = 0.0f;
    float lrg_size,lrg_pos,lrg = 0.0f;

    if (size.x > size_to.x) {new_w = size.x - size_to.x; new_w_positive = false;} 
    else {new_w = size_to.x - size.x; new_w_positive = true;}
    if (size.y > size_to.y) {new_h = size.y - size_to.y; new_h_positive = false;}
    else {new_h = size_to.y - size.y;new_h_positive = true;}
    if (pos.x > pos_to.x) {new_x = pos.x - pos_to.x; new_x_positive = false;}
    else {new_x = pos_to.x - pos.x;new_x_positive = true;}
    if (pos.y > pos_to.y) {new_y = pos.y - pos_to.y; new_y_positive = false;} 
    else {new_y = pos_to.y - pos.y;new_y_positive = true;}

    if (new_w > new_h) lrg_size = new_w;
    else lrg_size = new_h;
    if (new_x > new_y) lrg_pos = new_x;
    else lrg_pos = new_y;
    if (lrg_size > lrg_pos) lrg = lrg_size;
    else lrg = lrg_pos;

    if (new_w_positive) {if (btn_w < size_to.x) btn_w += (new_w / lrg) * global * io.DeltaTime;}
    else {if (btn_w > size_to.x) btn_w -= (new_w / lrg) * global * io.DeltaTime;}
    if (new_h_positive) {if (btn_h < size_to.y) btn_h += (new_h / lrg) * global * io.DeltaTime;} 
    else {if (btn_h > size_to.y) btn_h -= (new_h / lrg) * global * io.DeltaTime;}
    if (new_x_positive) {if (btn_x < pos_to.x) btn_x += (new_x / lrg) * global * io.DeltaTime;} 
    else {if (btn_x > pos_to.x) btn_x -= (new_x / lrg) * global * io.DeltaTime;}
    if (new_y_positive) {if (btn_y < pos_to.y) btn_y += (new_y / lrg) * global * io.DeltaTime;} 
    else {if (btn_y > pos_to.y) btn_y -= (new_y / lrg) * global * io.DeltaTime;}
}

void anim_box(const char* id, ImVec2 size, ImVec2 size_to, ImVec2 pos, ImVec2 pos_to) {
    ImGuiIO &io = ImGui::GetIO();
    float global = 500.0f;
    bool new_w_positive,new_h_positive,new_x_positive,new_y_positive = false;
    float new_w,new_h,new_x,new_y = 0.0f;
    float lrg_size,lrg_pos,lrg = 0.0f;

    if (size.x > size_to.x) {new_w = size.x - size_to.x; new_w_positive = false;} 
    else {new_w = size_to.x - size.x; new_w_positive = true;}
    if (size.y > size_to.y) {new_h = size.y - size_to.y; new_h_positive = false;}
    else {new_h = size_to.y - size.y;new_h_positive = true;}
    if (pos.x > pos_to.x) {new_x = pos.x - pos_to.x; new_x_positive = false;}
    else {new_x = pos_to.x - pos.x;new_x_positive = true;}
    if (pos.y > pos_to.y) {new_y = pos.y - pos_to.y; new_y_positive = false;} 
    else {new_y = pos_to.y - pos.y;new_y_positive = true;}

    if (new_w > new_h) lrg_size = new_w;
    else lrg_size = new_h;
    if (new_x > new_y) lrg_pos = new_x;
    else lrg_pos = new_y;
    if (lrg_size > lrg_pos) lrg = lrg_size;
    else lrg = lrg_pos;

    if (new_w_positive) {if (box_w < size_to.x) box_w += (new_w / lrg) * global * io.DeltaTime;}
    else {if (box_w > size_to.x) box_w -= (new_w / lrg) * global * io.DeltaTime;}
    if (new_h_positive) {if (box_h < size_to.y) box_h += (new_h / lrg) * global * io.DeltaTime;} 
    else {if (box_h > size_to.y) box_h -= (new_h / lrg) * global * io.DeltaTime;}
    if (new_x_positive) {if (box_x < pos_to.x) box_x += (new_x / lrg) * global * io.DeltaTime;} 
    else {if (box_x > pos_to.x) box_x -= (new_x / lrg) * global * io.DeltaTime;}
    if (new_y_positive) {if (box_y < pos_to.y) box_y += (new_y / lrg) * global * io.DeltaTime;} 
    else {if (box_y > pos_to.y) box_y -= (new_y / lrg) * global * io.DeltaTime;}
}

static inline ImVec2 operator+(const ImVec2 &lhs, const ImVec2 &rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle) {
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    ImVec2 pos[4] = {
        center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)};

    ImVec2 uvs[4] = {
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 0.0f),
        ImVec2(1.0f, 1.0f),
        ImVec2(0.0f, 1.0f)};

    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
}

void rotate_image(const char *image, ImVec2 pos, ImVec2 size, float angle, float speed) {
    ImGuiIO &io = ImGui::GetIO();
    ImTextureID image_texture = HelloImGui::ImTextureIdFromAsset(image);

    static float image_angle = angle;
    image_angle += io.DeltaTime * speed;
    ImVec2 image_center = ImVec2(pos.x, pos.y);

    ImGui::SetCursorPos(pos);
    ImageRotated(image_texture, image_center, size, image_angle);
}
