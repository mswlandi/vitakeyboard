#include <psp2kern/kernel/modulemgr.h>
#include "hidkeyboard.h"
#include "uapi/hidkeyboard_uapi.h"
#include <psp2kern/kernel/cpu.h> 

// static int hasPendingKey = 0;
// static char pendingKey = 0x10;

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start (SceSize args, void *argp)
{
    int ret;

    ret = hidkeyboard_module_start();
    if (ret < 0) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop (SceSize args, void *argp)
{
    hidkeyboard_module_stop();
    return SCE_KERNEL_STOP_SUCCESS;
}

int HidKeyboardSendKey(void)
{
    int state = 0;

    ENTER_SYSCALL(state);

    EXIT_SYSCALL(state);

    return 0;
}