#ifndef DESIGN_H
#define DESIGN_H

#define set_pos(a,b) ImGui::SetCursorPos(ImVec2(a,b))
#define font(a) ImGui::PushFont(io.Fonts->Fonts[a])
#define btn_text_on colors[ImGuiCol_Text] = ImVec4{.05f, .05f, .05f, 1.0f}
#define btn_text_off colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f}
#define dfu_text_light colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f}
#define dfu_text_dark colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 0.3f}
#define size(x,y) ImVec2(x,y)
#define pos(x,y) ImVec2(x,y)

/* Fonts we have loaded into memory */
#define SF_Reg 0 
#define SF_Med 2
#define SF_Bold 1 
#define SF_Sm_Bold 3
#define Hack 4
#define SF_RoundBold 5
#define SF_RegMed 6
#define SF_BoldLarge 7
#define WHITE ImVec4{1.0f, 1.0f, 1.0f, 1.0f}

void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle);
void draw_box(ImVec2 location, ImVec2 size, ImVec4 color, float radius);
void reset_design(void);
void reset_views(void);
bool is_hovered(const char* id, int w, int h);
bool is_pressed(const char* id, int w, int h);
void anim_btn(const char* id, ImVec2 size, ImVec2 size_to, ImVec2 pos, ImVec2 pos_to);
void anim_box(const char* id, ImVec2 size, ImVec2 size_to, ImVec2 pos, ImVec2 pos_to);
void label(const char *text, ImVec2 location, ImVec4 color, int font);
void rotate_image(const char *image, ImVec2 pos, ImVec2 size, float angle, float speed);
std::string addstr2(const char *str1, const char *str2);
std::string addstr3(const char *str1, const char *str2, const char *str3);
std::string addstr4(const char *str1, const char *str2, const char *str3, const char *str4);

void image_btn_style_alt(void);
void init_style(void);
void init_colors(void);
ImVec4 hex_color(int hex, float alpha);
ImColor hex_imcolor(int hex, float alpha);
void image_btn_style();

extern float btn_w;
extern float btn_h;
extern float btn_x;
extern float btn_y;

extern float box_w;
extern float box_h;
extern float box_x;
extern float box_y;

#endif