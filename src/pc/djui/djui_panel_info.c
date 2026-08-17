#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "pc/lua/utils/smlua_misc_utils.h"

static char sInfo[512] = { 0 };

void djui_panel_info_create(struct DjuiBase *caller) {
    struct DjuiThreePanel *panel = djui_panel_menu_create(DLANG(INFORMATION, INFORMATION_TITLE), false);
    struct DjuiBase *body = djui_three_panel_get_body(panel);
    {
        snprintf(sInfo, 512, "\
 Super Mario 64 OMM-Coop is an online multiplayer project for the Super Mario 64 PC port.\n\
 It is based on sm64coopdx, created by the Coop Deluxe Team, and integrates the native captures and content of the OMM (Outward-Mario-Moves) project.\n\
 More features, customization, and power to the Lua API allow modders and players to enjoy Super Mario 64 more than ever!");

        struct DjuiText* text = djui_text_create(body, sInfo);
        djui_base_set_location(&text->base, 0, 0);
        djui_base_set_size(&text->base, (DJUI_DEFAULT_PANEL_WIDTH * (configDjuiThemeCenter ? DJUI_THEME_CENTERED_WIDTH : 1)) - 64, 300);
        djui_base_set_color(&text->base, 220, 220, 220, 255);
        djui_text_set_drop_shadow(text, 64, 64, 64, 100);
        djui_text_set_alignment(text, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }

    djui_panel_add(caller, panel, NULL);
}
