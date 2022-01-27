
#ifndef __HIDKEYBOARD_H
#define __HIDKEYBOARD_H

#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/ctrl.h>
#include <psp2kern/udcd.h>

#include "log.h"
#include "uapi/hidkeyboard_uapi.h"
#include "usb_descriptors.h"

extern int hidkeyboard_module_start(void);
extern int hidkeyboard_module_stop(void);

#endif /* __HIDKEYBOARD_H */
