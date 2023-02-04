#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include <common.h>
#include <tui.h>

void bitfield_check_cb(newtComponent box, void* data) {
    if (newtCheckboxGetValue(box) == ' ') {
        *((tui_bit_info_t*)data)->flags_p = *((tui_bit_info_t*)data)->flags_p &~ ((tui_bit_info_t*)data)->opt;
    } else if (newtCheckboxGetValue(box) == '*') {
        *((tui_bit_info_t*)data)->flags_p |= ((tui_bit_info_t*)data)->opt;
    } else assert(0);
}

tui_screen_t tui_screen_options() {
    tui_bit_info_t verbose_info = { &kpf_flags, checkrain_option_verbose_boot };
    tui_bit_info_t rootful_info = { &palerain_flags, palerain_option_rootful };
    tui_bit_info_t setup_rootful_info = { &palerain_flags, palerain_option_setup_rootful };
    tui_bit_info_t force_revert_info = { &checkrain_flags, checkrain_option_force_revert };
    tui_bit_info_t safemode_info = { &checkrain_flags, checkrain_option_safemode };

    int ret = MAIN_SCREEN;

    newtCenteredWindow(60, 16, "palera1n version 2.0.0");
    newtComponent backButton = newtCompactButton(50, 14, "Back");

    newtComponent verboseBootBox = newtCheckbox(1, 1, "Verbose boot", CHECKBOX_STATE(kpf_flags, checkrain_option_verbose_boot), NULL, NULL);
    newtComponent rootfulBox = newtCheckbox(1, 2, "Rootful", CHECKBOX_STATE(palerain_flags, palerain_option_rootful), NULL, NULL);
    newtComponent rootfulSetupBox = newtCheckbox(1, 3, "Setup fakefs", CHECKBOX_STATE(palerain_flags, palerain_option_setup_rootful), NULL, NULL);
    newtComponent forceRevertBox = newtCheckbox(1, 4, "Restore system", CHECKBOX_STATE(checkrain_flags, checkrain_option_force_revert), NULL, NULL);
    newtComponent safeModeBox = newtCheckbox(1, 5, "Safe mode", CHECKBOX_STATE(checkrain_flags, checkrain_option_safemode), NULL, NULL);

    newtComponentAddCallback(verboseBootBox, bitfield_check_cb, &verbose_info);
    newtComponentAddCallback(rootfulBox, bitfield_check_cb, &rootful_info);
    newtComponentAddCallback(rootfulSetupBox, bitfield_check_cb, &setup_rootful_info);
    newtComponentAddCallback(forceRevertBox, bitfield_check_cb, &force_revert_info);
    newtComponentAddCallback(safeModeBox, bitfield_check_cb, &safemode_info);

    newtComponent form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, backButton, verboseBootBox, rootfulBox,
     rootfulSetupBox, forceRevertBox, safeModeBox, NULL);
    newtRunForm(form);
    newtRefresh();
    newtComponent buttonPressed = newtFormGetCurrent(form);
    if (buttonPressed == backButton) {
        ret = MAIN_SCREEN;
    } else assert(0);
    return ret;
}
