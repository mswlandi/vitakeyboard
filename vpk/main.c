#include <stdio.h>
#include <psp2/ctrl.h>
#include <taihen.h>
#include "debugScreen.h"
#include <psp2/sysmodule.h>
#include <psp2/libime.h>

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

static SceUID modid;
SceUInt32 libime_work[SCE_IME_WORK_BUFFER_SIZE / sizeof(SceInt32)];
SceWChar16 libime_out[SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1];
char libime_initval[8] = { 1 };
SceImeCaret caret_rev;

static void WaitKeyPress();
SceUID LoadModule(const char *path, int flags, int type);
void ImeEventHandler(void* arg, const SceImeEventData* e);

int main (int argc, char **argv)
{
    int res;

    psvDebugScreenInit();

    res = HidKeyboardInit();
    if (res < 0) {
        return -1;
    }

    SceCtrlData pad;
    printf("Press START to exit.\n");
    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START)
            break;
        sceKernelDelayThread(16 * 1000); // about 60 fps
    }

    if (modid >= 0) {
        taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
    }

    printf ("OK\n");
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

int HidKeyboardInit()
{
    int res, status;

    printf("Loading hidkeyboard module... ");
    modid = LoadModule("ux0:/tai/hidkeyboard.skprx", 0, 0);
    if (modid < 0) {
        printf("failed with 0x%08X\n", modid);
        printf("Place the skprx at ux0:/tai/hidkeyboard.skprx\n", modid);
        WaitKeyPress();
        return -1;
    }
    printf("OK\n");

    printf("Starting hidkeyboard module... ");
    res = taiStartKernelModule(modid, 0, NULL, 0, NULL, &status);
    if (res < 0) {
        printf("failed with 0x%08X\n", res);
        WaitKeyPress();
        return -1;
    }

    if (status) {
        printf("failed with status = 0x%08X\n", status);
        WaitKeyPress();
        return -1;
    }
}

SceUID LoadModule(const char* path, int flags, int type)
{
    return taiLoadKernelModule(path, 0, NULL);
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