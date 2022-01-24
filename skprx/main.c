#include <psp2kern/kernel/modulemgr.h>
#include "hidkeyboard.h"
#include "uapi/hidkeyboard_uapi.h"

// static int hasPendingKey = 0;
// static char pendingKey = 0x10;
// int mtx_lock;
// int res_mtx;

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start (SceSize args, void *argp)
{
    int ret;

    log_reset();

    ret = TEST_CALL(keyboard_start);
    // ret = keyboard_start();
    if (ret < 0) return ret;

    log_flush();

    // mtx_lock = ksceKernelCreateMutex("global_key_mutex", 0, 0, NULL);
    // if (mtx_lock < 0) return mtx_lock;

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop (SceSize args, void *argp)
{
    keyboard_stop ();
    return SCE_KERNEL_STOP_SUCCESS;
}

int HidKeyboardSendKey(void)
{
    // res_mtx = ksceKernelLockMutex(mtx_lock, 1, NULL);
    // if (res_mtx < 0) return res_mtx;

    // res_mtx = ksceKernelUnlockMutex(mtx_lock, 1);
    // if (res_mtx < 0) return res_mtx;
    
    return 1;
}