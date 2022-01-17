#include <psp2kern/kernel/modulemgr.h>
#include "hidkeyboard.h"

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start (SceSize args, void *argp)
{
  int ret;

  ret = keyboard_start ();
  if (ret < 0) return ret;

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop (SceSize args, void *argp)
{
  keyboard_stop ();
  return SCE_KERNEL_STOP_SUCCESS;
}
