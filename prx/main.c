
#include <pspkernel.h>
#include "hidmouse.h"

PSP_MODULE_INFO ("hidmouse", PSP_MODULE_KERNEL, 1, 1);

int module_start (SceSize args, void *argp)
{
  int ret;

  ret = mouse_start ();
  if (ret < 0) return ret;

  return 0;
}

int module_stop (SceSize args, void *argp)
{
  mouse_stop ();
  return 0;
}
