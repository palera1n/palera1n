#include <thread>
#include <iostream>
#include <math.h>
#include<unistd.h>

#include <imgui/imgui_internal.h>
#include <hello_imgui/hello_imgui.h>
#include <hello_imgui/app_window_params.h>

#include <design.h>
#include <common.h>
extern "C" {
#include <gui.h>
}

/*    Bools for which view to show    */
bool waiting_for_device = true;
bool unsupported_device = false;
bool supported_device = false;
bool recovery_wait = false;
bool jailbreak = false;
bool enter_dfu = false;
bool dfu_helper = false;
bool settings = false;
bool execjb = false;

uint64_t ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

/* -----------------  [  Main Function  ]  ----------------- */

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        // temp for testing
        return palera1n(argc, argv);
    }
   
    dfuhelper();

    /*    Vars for DFU Healper    */
    int dfu_step1 = 3;
    int dfu_step2 = 5;
    int dfu_step3 = 10;
    bool dfu_done = false;
    bool set_ms = true;
    auto desired = ms() + 1000;

    /*    HelloImGui Stuff    */
    HelloImGui::RunnerParams params;
    params.fpsIdling.fpsIdle = 60.f;
    params.appWindowParams.borderless = false;
    params.appWindowParams.resizable = false;
    params.appWindowParams.windowGeometry.size = {550, 300};
    params.appWindowParams.windowTitle = "palera1n";
    params.appWindowParams.restorePreviousGeometry = false;
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;

    /*    Color & Size Stuff    */
    bool open = true;
    ImColor grad_color_left = ImColor(255, 177, 35);
    ImColor grad_color_right = ImColor(4, 121, 168);

    int grad_color_left_r = 255;
    float text_alpha = 0.0f;
    bool reverse_gradient = false;


    /*    ImGui Loop    */
    params.callbacks.ShowGui = [&]() {
        {
            /*    Init colors, style, and fonts    */
            auto &colors = ImGui::GetStyle().Colors;
            auto &style = ImGui::GetStyle();
            ImGuiIO &io = ImGui::GetIO();
            init_colors();
            init_style();

            /*    Init window size and start ImGui    */
            ImGui::SetNextWindowSize(ImVec2(560, 300));
            ImGui::SetNextWindowPos(ImVec2(-5, 0));
            ImGui::Begin("palera1n", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

            /*    Set our background gradient   */
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (!reverse_gradient) grad_color_left_r -= FLOAT_TO_INT(io.DeltaTime * 100.f) /2;
            else grad_color_left_r += FLOAT_TO_INT(io.DeltaTime * 100.f) /2;
            if (grad_color_left_r <= 140) reverse_gradient = true;
            if (grad_color_left_r >= 255) reverse_gradient = false;
            grad_color_left = ImColor(135, 63, grad_color_left_r) ;
            grad_color_right = ImColor(4, 160, grad_color_left_r) ;

            auto dl(ImGui::GetWindowDrawList());
            dl->AddRectFilledMultiColor(ImVec2(0, 0), // pos - x y
                                        ImVec2(560, 300), // size - width height
                                        grad_color_left, // color top left
                                        hex_imcolor(0x2390b4, 1.0f), // color 
                                        hex_imcolor(0x04c2a8, 1.0f), // color
                                        grad_color_right); // color


            if (!recovery_wait && !dfu_helper && !jailbreak) {
                ImTextureID settings_icon = HelloImGui::ImTextureIdFromAsset("s60.png"); // pngs will be embedded soon
                ImTextureID about_icon = HelloImGui::ImTextureIdFromAsset("i60.png"); // pngs will be embedded soon
                image_btn_style_alt();
                ImGui::SetCursorPos(ImVec2(455, 245));
                ImGui::ImageButton(about_icon, ImVec2(33.0f, 33.0f));
                ImGui::SetCursorPos(ImVec2(500, 245));
                ImGui::ImageButton(settings_icon, ImVec2(33.0f, 33.0f)); 
                reset_design();
            }

            /*    Waiting for device view    */
            if (waiting_for_device) {
                if (device_status == NORMAL) {
                    const char *name = dev.displayName;
                    char *arch = dev.CPUArchitecture;
                    char *type = dev.productType;
                    char *version = dev.productVersion;
                    if (strcmp("arm64", arch) != 0) {
                        waiting_for_device = false;
                        unsupported_device = true;
                        btn_x = 213.0f;
                        btn_y = 190.0f;
                        btn_w = 125.0f;
                        btn_h = 35.0f;
                    } else if (strstr(version, "15") != 0 || strstr(version, "16") != 0) {        
                        waiting_for_device = false;
                        supported_device = true;
                        btn_x = 213.0f;
                        btn_y = 190.0f;
                        btn_w = 125.0f;
                        btn_h = 35.0f;
                    }
                }
                box_x = 55.0f; box_y = 75.0f; box_w = 450.0f; box_h = 150.0f;

                draw_box(pos(55, 75), size(450, 150), hex_color(0x000000, 0.25f), 20.0f); // box in center
                rotate_image("spinner.png", pos(95, 150), size(40, 40), 0.0f, 4.0f); // loading anim in box
                label("Waiting for connection...", pos(135,122), WHITE, SF_BoldLarge); // Large title
                label("Connect your device in normal mode to get started!", pos(138, 157), WHITE, SF_Reg); // Sub title


            /*    Unsupported device view    */
            } else if (unsupported_device) {
              
                anim_box("Jailbreak", ImVec2(450,150),ImVec2(300,200),ImVec2(54,75),ImVec2(129,50));
                draw_box(ImVec2(box_x, box_y),ImVec2(box_w, box_h), hex_color(0x000000, 0.25f), 20.0f);
   
                set_pos(167, 90); font(SF_Bold);
                ImGui::Text("%s", dev.displayName);
                ImGui::PopFont();

                set_pos(167, 120); font(SF_RegMed);
                ImGui::Text("%s running iOS %s (%s)", dev.productType, dev.productVersion, dev.CPUArchitecture);
                
                colors[ImGuiCol_Text] = hex_color(0xEB5160, 1.0f);
                set_pos(167, 147); ImGui::Text(ICON_FA_BAN);
                colors[ImGuiCol_Text] = hex_color(0xFFFFFF, 1.0f);

                ImGui::SameLine(); ImGui::Text("Unsupported device connected");
                ImGui::PopFont();
                
                set_pos(213, 190); font(SF_RoundBold); btn_text_on;
                bool hovered = is_hovered("Exit", 125, 35);
                bool pressed = is_pressed("Exit", 125, 35);
                ImVec2 btn_size = ImVec2(120, 30);

                if (hovered) {
                    anim_btn("Exit", ImVec2(125,35),ImVec2(127,37),ImVec2(213,190),ImVec2(212,189));
                    btn_size = ImVec2(btn_w, btn_h);
                    ImGui::SetCursorPos(ImVec2(btn_x,btn_y));
                } else {
                    anim_btn("Exit", ImVec2(127,37),ImVec2(125,35),ImVec2(212,189),ImVec2(213,190));
                    btn_size = ImVec2(btn_w, btn_h);
                    ImGui::SetCursorPos(ImVec2(btn_x,btn_y));
                }
                
                ImGui::Button("Exit", btn_size);
                if (pressed) {
                    exit(0);
                }


            /*    Supported device view     */
            } else if (supported_device) {
                anim_box("Jailbreak", ImVec2(450,150),ImVec2(300,200),ImVec2(54,75),ImVec2(129,50));
                draw_box(ImVec2(box_x, box_y),ImVec2(box_w, box_h), hex_color(0x000000, 0.25f), 20.0f);
   
                set_pos(167, 90); font(SF_Bold);
                ImGui::Text("%s", dev.displayName);
                ImGui::PopFont();

                set_pos(167, 120); font(SF_RegMed);
                ImGui::Text("%s running iOS %s (%s)", dev.productType, dev.productVersion, dev.CPUArchitecture);
                
                colors[ImGuiCol_Text] = hex_color(0x3CFF55, 1.0f);
                set_pos(167, 147); ImGui::Text(ICON_FA_CHECK_CIRCLE);
                colors[ImGuiCol_Text] = hex_color(0xFFFFFF, 1.0f);

                ImGui::SameLine(); ImGui::Text("Connected and ready.");
                ImGui::PopFont();

                set_pos(213, 190); font(SF_RoundBold); btn_text_on;
                bool hovered = is_hovered("Jailbreak", 125, 35);
                bool pressed = is_pressed("Jailbreak", 125, 35);
                ImVec2 btn_size = ImVec2(120, 30);

                if (hovered) {
                    anim_btn("Jailbreak", ImVec2(125,35),ImVec2(127,37),ImVec2(213,190),ImVec2(212,189));
                    btn_size = ImVec2(btn_w, btn_h);
                    ImGui::SetCursorPos(ImVec2(btn_x,btn_y));
                } else {
                    anim_btn("Jailbreak", ImVec2(127,37),ImVec2(125,35),ImVec2(212,189),ImVec2(213,190));
                    btn_size = ImVec2(btn_w, btn_h);
                    ImGui::SetCursorPos(ImVec2(btn_x,btn_y));
                }
                
                ImGui::Button("Jailbreak", btn_size);
                if (pressed) {
                    enter_recovery_cmd(0);
                    box_x = 140.0f; box_y = 55.0f; box_w = 320.0f; box_h = 240.0f;
                    supported_device = false;
                    recovery_wait = true;
                    btn_text_off;
                    ImGui::PopFont();
                }


            /*    Waiting for recovery view     */
            } else if (recovery_wait) {

                draw_box(ImVec2(125, 105),ImVec2(300, 90), hex_color(0x000000, 0.25f), 20.0f);
                //ImageRotated(loader, ImVec2(160, 150), ImVec2(40.0f, 40.0f), angle);

                set_pos(210, 138); font(SF_Bold);
                ImGui::Text("Entering recovery...");
                ImGui::PopFont();

                if (device_status == RECOVERY) {
                    recovery_wait = false;
                    enter_dfu = true;
                }

            /*    Ready for DFU view     */
            } else if (enter_dfu) {

                draw_box(ImVec2(65, 105),ImVec2(450, 90), hex_color(0x000000, 0.25f), 20.0f);

                set_pos(110, 128); font(SF_Bold);
                ImGui::Text("Time to enter DFU mode!");
                ImGui::PopFont();

                set_pos(110, 153); font(SF_Reg);
                ImGui::Text("Locate the power and volume down button before you proceed.");

                set_pos(108, 210); font(SF_Sm_Bold); btn_text_on;
                if (ImGui::Button("Start", ImVec2(125, 35))) {
                    enter_dfu = false;
                    dfu_helper = true;
                    btn_text_off;
                    ImGui::PopFont();
                }

            /*    DFU helper view     */
            } else if (dfu_helper) {
                draw_box(ImVec2(75, 65),ImVec2(450, 220), hex_color(0x000000, 0.5f), 20.0f);

                if (dfu_step1 > 0) dfu_text_light;
                else dfu_text_dark;
                set_pos(110, 110); font(SF_Reg);
                ImGui::Text("1. Get Ready! %d", dfu_step1);

                if (dfu_step2 > 0 && dfu_step1 == 0) dfu_text_light;
                else dfu_text_dark;
                set_pos(110, 160); ImGui::Text("2. Hold volume down + side button %d", dfu_step2);

                if (dfu_step3 > 0 && dfu_step2 == 0) dfu_text_light;
                else dfu_text_dark;;
                set_pos(110, 210);ImGui::Text("3. Release side button, but keep holding volume down %d", dfu_step3);
                ImGui::PopFont();

                if (!dfu_done) {
                    if (set_ms) {
                        desired = ms() + 1000;
                        set_ms = false;
                    }
                    if (ms() >= desired) {
                        set_ms = true;
                    if (dfu_step1 > 0) dfu_step1 -= 1;
                    else if (dfu_step2 > 0) dfu_step2 -= 1;
                    else if (dfu_step3 > 0) dfu_step3 -= 1;
                    else dfu_done = true;
                    }
                }

                if (dfu_step2 == 2) exitrecv_cmd(dev.ecid);
                if (dfu_done) {
                    if (device_status == DFU) {
                        guifree();
                        dfu_helper = false;
                        jailbreak = true;
                    } else {
                        send_alert("Failed to enter DFU", "Device failed to enter DFU mode, please try again.");
                        dfu_step1 = 3; dfu_step2 = 5; dfu_step3 = 10;
                        dfu_done = false;
                        dfu_helper = false;
                        waiting_for_device = true;
                    }
                }

            /*    Main Jailbreak view     */
            } else if (jailbreak) {
	            pthread_t jb;

                draw_box(ImVec2(25, 70),ImVec2(500, 200), hex_color(0x000000, 0.25f), 20.0f);

                set_pos(20, 15); font(SF_Bold);
                ImGui::Text("Installing jailbreak...");
                ImGui::PopFont();

                set_pos(20, 40); font(SF_Reg);
                ImGui::Text("This will take a moment to jailbreak, hang tight!");

                set_pos(50, 100); font(Hack);
                if (!execjb) {
                    execjb = true;
                    palera1n(1, NULL);
                }
           
                // redirect logs here...
                ImGui::Text("[*] Running...");
                set_pos(50, 125); ImGui::Text("[*] Some epic log stuff will go here :3");
                set_pos(50, 150); ImGui::Text("[*] lorem ipsum :frcoal:");
                ImGui::PopFont();
                
            } else {
            }
            if (settings) {
                draw_box(ImVec2(50.0f, 65.0f),ImVec2(230.0f, 230.0f), hex_color(0x000000, 0.5f), 20.0f);
                draw_box(ImVec2(320.0f, 65.0f),ImVec2(230.0f, 230.0f), hex_color(0x000000, 0.5f), 20.0f);
            }
            ImGui::End();
        }
    };
    HelloImGui::Run(params);
    return 0;
}
