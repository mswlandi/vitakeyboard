#include <stdio.h>
#include <psp2/ctrl.h>
#include <taihen.h>
#include "debugScreen.h"

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

static SceUID modid;

static void wait_key_press();

SceUID load_module (const char *path, int flags, int type)
{
  return taiLoadKernelModule (path, 0, NULL);
}

int main (int argc, char **argv)
{
  int res, status;

  psvDebugScreenInit();

  printf ("Loading modules... ");
  modid = load_module ("ux0:/data/hidmouse.skprx", 0, 0);
  if (modid < 0) {
    printf ("failed with 0x%08X\n", modid);
    printf ("Place the skprx at ux0:/data/hidmouse.skprx\n", modid);
    wait_key_press();
    return -1;
  }

  printf ("OK\n");
  printf ("Starting modules... ");

  res = taiStartKernelModule  (modid, 0, NULL, 0, NULL, &status);
  if (res < 0) {
    printf ("failed with 0x%08X\n", res);
    wait_key_press();
    return -1;
  }

  if (status) {
    printf ("failed with status = 0x%08X\n", status);
    wait_key_press();
    return -1;
  }

  wait_key_press();

  if (modid >= 0) {
    taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
  }

  printf ("OK\n");
  return 0;
}

void wait_key_press()
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

