#include <stdio.h>
#include <psp2/ctrl.h>
#include <psp2/sysmodule.h>
#include <psp2/libime.h>
#include <psp2/kernel/error.h>
#include <psp2/appmgr.h>
#include "hidkeyboard_uapi.h"
#include "layouts.h"
#include "debugScreen.h"

#define printf psvDebugScreenPrintf
#define SAVEFILEPATH "ux0:/data/vitakeyboard_savefile.bin"

SceUInt32 libime_work[SCE_IME_WORK_BUFFER_SIZE / sizeof(SceInt32)];
SceWChar16 libime_out[SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1];
char libime_initval[8] = { 1 };
SceImeCaret caret_rev;
int ime_just_closed = 0;

int layout_choice = 0;

static void WaitKeyPress();
SceUID LoadModule(const char *path, int flags, int type);
void ImeEventHandler(void* arg, const SceImeEventData* e);
void DebugScreenInit();
int ImeInit();
int HidKeyboardInit();
void LoadSavedLayoutChoice();
void SaveLayoutChoice();

int main (int argc, char **argv)
{
    int ret;
    SceCtrlData pad;

    DebugScreenInit();

    printf("VitaKeyboard by MarkOfTheLand\n");

    // stops before starting in case app was reset
    hidkeyboard_user_stop();
    ret = hidkeyboard_user_start();
    if (ret >= 0) {
        printf("hidkeyboard started successfully!\n");
    }
    else if (ret == HIDKEYBOARD_ERROR_DRIVER_ALREADY_ACTIVATED) {
        printf("hidkeyboard is already active!\n");
    }
    else if (ret < 0) {
        printf("Error initializing the module: 0x%08X\n", ret);

        // Common error
        if (ret == 0x80243002) {
            printf("Beware that VitaKeyboard is not compatible with UDCD_UVC or other USB device emulation plugins.\n");
        }

        WaitKeyPress();
        return -1;
    }

    printf("\nControls: (with the virtual keyboard closed)\n");
    printf("- START:        properly close the application\n");
    printf("- TRIANGLE:     reopen the virtual keyboard\n");
    printf("- UP, DOWN, X:  select layout to emulate\n");

    ret = ImeInit();
    if (ret < 0) {
        printf("Failed opening virtual keyboard.\n");
        WaitKeyPress();
        return -1;
    }

    LoadSavedLayoutChoice();

    int layout_sel_cursor = layout_choice;
    int is_updown_pressed = 0;
    int is_x_pressed = 0;

    while (1) {

        printf("\e[s"); // Saves cursor position
        printf("\nChoose a layout:\n");

        for (int i = 0; i < sizeof(layout_list) / sizeof(layout_list_entry); i++) {
            if (layout_sel_cursor == i) {
                printf("-> ");
            }
            else {
                printf("   ");
            }

            printf("%s", layout_list[i].layout_name);

            if (layout_choice == i) {
                printf(" [SELECTED]\n");
            }
            else {
                printf("           \n");
            }
        }

        printf("\e[u"); // Return to saved cursor position

        sceCtrlPeekBufferPositive(0, &pad, 1);
        // UP: make selection of layout go up
        if (pad.buttons & SCE_CTRL_UP && !is_updown_pressed) {
            layout_sel_cursor -= 1;
            if (layout_sel_cursor < 0) {
                layout_sel_cursor = sizeof(layout_list) / sizeof(layout_list_entry) - 1;
            }
            is_updown_pressed = 1;
        }
        // DOWN: make selection of layout go down
        else if (pad.buttons & SCE_CTRL_DOWN && !is_updown_pressed) {
            layout_sel_cursor += 1;
            if (layout_sel_cursor >= sizeof(layout_list) / sizeof(layout_list_entry)) {
                layout_sel_cursor = 0;
            }
            is_updown_pressed = 1;
        }
        else if (!(pad.buttons & SCE_CTRL_DOWN) && !(pad.buttons & SCE_CTRL_UP)) {
            is_updown_pressed = 0;
        }

        // CROSS: select layout
        if (pad.buttons & SCE_CTRL_CROSS && !is_x_pressed) {
            layout_choice = layout_sel_cursor;
            SaveLayoutChoice();
            is_x_pressed = 1;
        }
        else if (!(pad.buttons & SCE_CTRL_CROSS) && is_x_pressed) {
            is_x_pressed = 0;
        }

        // TRIANGLE: reopen the IME if it is closed
        if (pad.buttons & SCE_CTRL_TRIANGLE) {
            ret = ImeInit();
            if (ret < 0 && ret != SCE_IME_ERROR_ALREADY_OPENED) {
                printf("Error on opening virtual keyboard: 0x%08X\n", ret);
                WaitKeyPress();
                break;
            }
        }

        // START: close the app
        if (pad.buttons & SCE_CTRL_START) {
            break;
        }
        sceKernelDelayThread(10 * 1000); // about 100 fps

        sceImeUpdate();
    }

    hidkeyboard_user_stop();

    return 0;
}

void WaitKeyPress()
{
    SceCtrlData pad;

    printf("Press START to exit.\n");

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START)
        break;
        sceKernelDelayThread(100 * 1000);
    }
}

void DebugScreenInit()
{
    PsvDebugScreenFont* psvDebugScreenFont_default_1x;
    PsvDebugScreenFont* psvDebugScreenFont_default_2x;

    psvDebugScreenInit();

    psvDebugScreenFont_default_1x = psvDebugScreenGetFont();
    psvDebugScreenFont_default_2x = psvDebugScreenScaleFont2x(psvDebugScreenFont_default_1x);
    if (!psvDebugScreenFont_default_2x) {
        psvDebugScreenFont_default_2x = psvDebugScreenFont_default_1x;
    }
    else {
        psvDebugScreenSetFont(psvDebugScreenFont_default_2x);
    }

    psvDebugScreenSetFgColor(0xEEEEEE);
}

int ImeInit()
{
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);

    memset(libime_out, 0, ((SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1) * sizeof(SceWChar16)));

    SceImeParam param;
    SceInt32 res;

    sceImeParamInit(&param);
    param.supportedLanguages = 0;
    param.languagesForced = SCE_TRUE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = SCE_IME_OPTION_NO_ASSISTANCE | SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
    param.inputTextBuffer = libime_out;
    param.maxTextLength = SCE_IME_MAX_TEXT_LENGTH;
    param.handler = ImeEventHandler;
    param.filter = NULL;
    param.initialText = (SceWChar16*)libime_initval;
    param.arg = NULL;
    param.work = libime_work;
    res = sceImeOpen(&param);

    return res;
}

void ImeEventHandler(void* arg, const SceImeEventData* e)
{
    switch (e->id) {
    case SCE_IME_EVENT_UPDATE_TEXT:
        if (e->param.text.caretIndex == 0) {
            // when you reopen the ime keyboard it updates text with an empty initial value
            if (ime_just_closed) {
                ime_just_closed = 0;
            }
            else {
                // backspace
                HidKeyBoardSendModifierAndKey(0x00, 0x2a);
                sceImeSetText((SceWChar16*)libime_initval, 4);
            }
        }
        else {
            // new character
            utf16_to_hid_mapping map;
            map = getLayoutMappingFromUtf16((unsigned short int)libime_out[1], layout_list[layout_choice].layout, layout_list[layout_choice].num_mappings_of_layout);

            HidKeyBoardSendModifierAndKey(map.hid_modifiers1, map.hid_key1);

            if (map.hid_key2 != 0x00) {
                sceKernelDelayThread(10 * 1000); // 0.01s
                HidKeyBoardSendModifierAndKey(map.hid_modifiers2, map.hid_key2);
            }

            memset(&caret_rev, 0, sizeof(SceImeCaret));
            memset(libime_out, 0, ((SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1) * sizeof(SceWChar16)));
            caret_rev.index = 1;
            sceImeSetCaret(&caret_rev);
            sceImeSetText((SceWChar16*)libime_initval, 4);
        }
        break;
    case SCE_IME_EVENT_PRESS_ENTER:
        // enter
        HidKeyBoardSendModifierAndKey(0x00, 0x58);
        break;
    case SCE_IME_EVENT_PRESS_CLOSE:
        ime_just_closed = 1;
        sceImeClose();
        break;
    }
}

void LoadSavedLayoutChoice()
{
    FILE* fd = fopen(SAVEFILEPATH, "rb");

    if (fd != NULL) {
        fread(&layout_choice, sizeof(int), 1, fd);
    }
    else {
        // Create and populate savedata if doesn't exist
        FILE* fd = fopen(SAVEFILEPATH, "wb");
        if (fd != NULL) {
            fwrite(&layout_choice, sizeof(int), 1, fd);
        }
        else {
            printf("\e[s"); // Saves cursor position
            printf("\e[15;25H" "error creating savefile");
            printf("\e[u"); // Return to saved cursor position
        }
    }

    fclose(fd);
}

void SaveLayoutChoice()
{
    FILE* fd = fopen(SAVEFILEPATH, "wb");

    if (fd != NULL) {
        fwrite(&layout_choice, sizeof(int), 1, fd);
    }
    else {
        printf("\e[s"); // Saves cursor position
        printf("\e[15;25H" "error saving on savefile");
        printf("\e[u"); // Return to saved cursor position
    }

    fclose(fd);
}