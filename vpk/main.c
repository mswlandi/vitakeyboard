#include <stdio.h>
#include <psp2/ctrl.h>
#include <taihen.h>
#include "debugScreen.h"
#include <psp2/sysmodule.h>
#include <psp2/libime.h>
#include <psp2/kernel/error.h> 
#include "hidkeyboard_uapi.h"

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

SceUInt32 libime_work[SCE_IME_WORK_BUFFER_SIZE / sizeof(SceInt32)];
SceWChar16 libime_out[SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1];
char libime_initval[8] = { 1 };
SceImeCaret caret_rev;

static void WaitKeyPress();
SceUID LoadModule(const char *path, int flags, int type);
void ImeEventHandler(void* arg, const SceImeEventData* e);
int ImeInit();
int HidKeyboardInit();

int main (int argc, char **argv)
{
    int ret;
    SceCtrlData pad;

    psvDebugScreenInit();

    printf("starting the module... ");

    ret = hidkeyboard_user_start();
    if (ret >= 0) {
        printf("hidkeyboard started successfully!\n");
    }
    else if (ret == HIDKEYBOARD_ERROR_DRIVER_ALREADY_ACTIVATED) {
        printf("hidkeyboard is already active!\n");
    }
    else if (ret < 0) {
        printf("Error hidkeyboard_user_start(): 0x%08X\n", ret);
        WaitKeyPress();
        return -1;
    }

    ret = ImeInit();
    if (ret < 0) {
        printf("Failed opening virtual keyboard.\n");
        WaitKeyPress();
        return -1;
    }

    printf("Press START to exit.\n");
    while (1) {

        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START) {
            HidKeyboardSendKey();
            break;
        }
        sceKernelDelayThread(16 * 1000); // about 60 fps

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

int ImeInit()
{
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);

    memset(libime_out, 0, ((SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1) * sizeof(SceWChar16)));

    SceImeParam param;
    SceInt32 res;

    sceImeParamInit(&param);
    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = SCE_IME_OPTION_NO_ASSISTANCE;
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
            // backspace
            sceImeSetText((SceWChar16*)libime_initval, 4);
        }
        else {
            // test for scancode from key
            // if not space, utf16_toutf8 to send text
            memset(&caret_rev, 0, sizeof(SceImeCaret));
            memset(libime_out, 0, ((SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1) * sizeof(SceWChar16)));
            caret_rev.index = 1;
            sceImeSetCaret(&caret_rev);
            sceImeSetText((SceWChar16*)libime_initval, 4);
        }
        break;
    case SCE_IME_EVENT_PRESS_ENTER:
        // enter
        break;
    case SCE_IME_EVENT_PRESS_CLOSE:
        sceImeClose();
        break;
    }
}