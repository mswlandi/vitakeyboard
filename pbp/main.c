#include <pspuser.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <string.h>

#define printf pspDebugScreenPrintf

/* Define the module info section, note the 0x1000 flag to enable start in kernel mode */
PSP_MODULE_INFO ("hidmouseuser", 0, 1, 1);

PSP_MAIN_THREAD_ATTR (PSP_THREAD_ATTR_USER);


static SceUID modid;
/* Exit callback */
int exit_callback (int arg1, int arg2, void *common)
{
  int status;
  printf ("Stopping modules...\n");
  sceKernelStopModule (modid, 0, NULL, &status, NULL);
  sceKernelUnloadModule (modid);
  sceKernelExitGame ();
  return 0;
}

/* Callback thread */
int CallbackThread (SceSize args, void *argp)
{
  int cbid;

  cbid = sceKernelCreateCallback ("Exit Callback", &exit_callback, NULL);
  sceKernelRegisterExitCallback (cbid);
  sceKernelSleepThreadCB ();

  return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks (void)
{
  int thid = 0;

  thid = sceKernelCreateThread ("update_thread", &CallbackThread,
				     0x11, 0xFA0, 0, 0);
  if (thid >= 0) {
    sceKernelStartThread (thid, 0, 0);
  }

  return thid;
}


SceUID load_module (const char *path, int flags, int type)
{
  SceKernelLMOption option;
  SceUID mpid;

  /* If the type is 0, then load the module in the kernel partition, otherwise load it in the user partition. */
  if (type == 0) {
    mpid = 1;
  } else {
    mpid = 2;
  }

  memset (&option, 0, sizeof (option));
  option.size = sizeof (option);
  option.mpidtext = mpid;
  option.mpiddata = mpid;
  option.position = 0;
  option.access = 1;
  return sceKernelLoadModule (path, flags, &option);
}

int main (int argc, char **argv)
{
  int res, status;

  SetupCallbacks ();
  pspDebugScreenInit ();
  
  printf ("Loading modules... ");
  modid = load_module ("hidmouse.prx", 0, 0);
  if (modid < 0) {
    printf ("failed with 0x%08X\n", modid);
    return -1;
  }

  printf ("OK\n");
  printf ("Starting modules... ");

  res = sceKernelStartModule (modid, 0, NULL, &status, NULL);
  if (res < 0) {
    printf ("failed with 0x%08X\n", res);
    return -1;
  }

  if (status) {
    printf ("failed with status = 0x%08X\n", status);
    return -1;
  }

  printf ("OK\n");
  return 0;
}
