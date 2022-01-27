#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/ctrl.h>
#include <psp2kern/udcd.h>

#include "log.h"
#include "uapi/hidkeyboard_uapi.h"
#include "usb_descriptors.h"

#define Kprintf(...) (void)0

#define VITA_USB_KEYBOARD                    "VITA_KEYBOARD"
#define VITA_USB_KEYBOARD_PID                0x1338

#define EVF_CONNECTED		    (1 << 0)
#define EVF_DISCONNECTED	    (1 << 1)
#define EVF_EXIT		        (1 << 2)
#define EVF_INT_REQ_COMPLETED	(1 << 3)
#define EVF_ALL_MASK		    (EVF_INT_REQ_COMPLETED | (EVF_INT_REQ_COMPLETED - 1))

static char g_inputs[8] __attribute__((aligned(64))) = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static struct SceUdcdDeviceRequest g_request;
static struct SceUdcdDeviceRequest g_reportrequest;
static SceUID g_thid = -1;
static int g_run = 1;

static SceUID usb_event_flag_id;
static int hidkeyboard_driver_registered = 0;
static int hidkeyboard_driver_activated = 0;

/* Forward define driver functions */
static int start_func(int size, void* args, void* user_data);
static int stop_func(int size, void* args, void* user_data);
static int usb_recvctl(int arg1, int arg2, struct SceUdcdEP0DeviceRequest* req, void* user_data);
static int usb_change(int interfaceNumber, int alternateSetting, int bus);
static int usb_attach(int usb_version, void* user_data);
static void usb_detach(void* user_data);
static void usb_configure(int usb_version, int desc_count, struct SceUdcdInterfaceSettings* settings, void* user_data);

/* USB host driver */
struct SceUdcdDriver g_driver =
{
    VITA_USB_KEYBOARD,  /* driverName */
    2,                  /* numEndpoints */
    &endpoints[0],      /* endpoints */
    &interfaces[0],     /* interface */
    &devdesc_hi,        /* descriptor_hi */
    &config_hi,         /* configuration_hi */
    &devdesc_full,      /* descriptor */
    &config_full,       /* configuration */
    &descriptors[0],    /* stringDescriptors */
    NULL,
    NULL,
    &usb_recvctl,       /* processRequest */
    &usb_change,        /* chageSetting */
    &usb_attach,        /* attach */
    &usb_detach,        /* detach */
    &usb_configure,     /* configure */
    &start_func,        /* start func */
    &stop_func,         /* stop func */
    0,
    0,
    NULL                /* link to driver */
};


static void send_inputs(void);

static
void complete_request(struct SceUdcdDeviceRequest* req)
{
    Kprintf("complete_request\n");
    req->unused = NULL;
}


/* Device request */
static
int usb_recvctl(int arg1, int arg2, struct SceUdcdEP0DeviceRequest* req, void* user_data)
{
    Kprintf("recvctl: %x %x\n", arg1, arg2);
    Kprintf("request: %x type: %x wValue: %x wIndex: %x wLength: %x\n",
        req->bRequest, req->bmRequestType, req->wValue, req->wIndex, req->wLength);

    if (req->bmRequestType == 0x81 && req->bRequest == 0x06 && req->wValue == 0x2200 && arg2 != -1) {
        if (!g_reportrequest.unused) {
            g_reportrequest.data = hid_report;
            g_reportrequest.size = sizeof(hid_report);
            g_reportrequest.endpoint = &endpoints[0];
            if (g_reportrequest.size > req->wLength)
                g_reportrequest.size = req->wLength;
            g_reportrequest.isControlRequest = 0;
            g_reportrequest.onComplete = &complete_request;
            g_reportrequest.transmitted = 0;
            g_reportrequest.returnCode = 0;
            g_reportrequest.unused = &g_reportrequest;
            g_reportrequest.next = NULL;
            g_reportrequest.physicalAddress = NULL;
            // ksceUdcdReqSend (&g_reportrequest);
            TEST_CALL(ksceUdcdReqSend, &g_reportrequest);
        }
    }
    return 0;
}

/* Alternate settings */
static
int usb_change(int interfaceNumber, int alternateSetting, int bus)
{
    Kprintf("usb_change %d %d\n", interfaceNumber, alternateSetting);
    return 0;
}

/* Attach callback */
static
int usb_attach(int usb_version, void* user_data)
{
    Kprintf("usb_attach %d\n", usb_version);
    return 0;
}

/* Detach callback */
static
void usb_detach(void* user_data)
{
    Kprintf("usb_detach\n");
}

static
void usb_configure(int usb_version, int desc_count, struct SceUdcdInterfaceSettings* settings, void* user_data)
{
    Kprintf("usb_configure %d %d %p %d\n", usb_version, desc_count, settings, settings->numDescriptors);
}

/* USB start function */
static
int start_func(int size, void* p, void* user_data)
{
    Kprintf("start\n");
    return 0;
}

/* USB stop function */
static
int stop_func(int size, void* p, void* user_data)
{
    Kprintf("stop\n");
    return 0;
}

static
void send_inputs(void)
{
    if (!g_request.unused) {
        g_request.endpoint = &endpoints[1];
        g_request.data = g_inputs;
        g_request.size = sizeof(g_inputs);
        g_request.isControlRequest = 0;
        g_request.onComplete = &complete_request;
        g_request.transmitted = 0;
        g_request.returnCode = 0;
        g_request.unused = &g_request;
        g_request.next = NULL;
        g_request.physicalAddress = NULL;
        ksceUdcdReqSend(&g_request);
    }
}

static
int update_keyboard(SceSize args, void* argp)
{
    SceCtrlData pad;
    int pressed = 0;
    int changed = 0;

    //ksceCtrlSetSamplingCycle (0);
    ksceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    while (g_run) {

        ksceCtrlPeekBufferPositive(0, &pad, 1);

        // if (hasPendingKey && !pressed) {
        //     hasPendingKey = 0;
        //     g_inputs[3] = pendingKey;
        //     pressed = 1;
        //     changed = 1;
        // }
        // else if (pressed) {
        //     g_inputs[3] = 0x00;
        //     pressed = 0;
        //     changed = 1;
        // }

        if (pad.buttons & SCE_CTRL_CROSS && !pressed)
        {
            g_inputs[3] = 0x10;
            pressed = 1;
            changed = 1;
        }
        else if (!(pad.buttons & SCE_CTRL_CROSS) && pressed)
        {
            g_inputs[3] = 0x00;
            pressed = 0;
            changed = 1;
        }

        ksceKernelCpuDcacheAndL2WritebackRange(g_inputs, sizeof(g_inputs));

        if (ksceUdcdGetDeviceState() & SCE_UDCD_STATUS_CONNECTION_ESTABLISHED && changed)
            send_inputs();
        ksceKernelDelayThread(10000);

        changed = 0;
    }
    return 0;
}

// static int hasPendingKey = 0;
// static char pendingKey = 0x10;

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start (SceSize args, void *argp)
{
    // if returns -1 SCE_KERNEL_START_FAILED
    int ret = 0;

    g_thid = ksceKernelCreateThread("update_thread", &update_keyboard, 0x3C, 0x1000, 0, 0x10000, 0);
    if (g_thid < 0) {
        goto err_return;
    }

    usb_event_flag_id = ksceKernelCreateEventFlag("hidkeyboard_event_flag", 0, 0, NULL);
    if (usb_event_flag_id < 0) {
        goto err_destroy_thread;
    }

    ret = ksceUdcdRegister(&g_driver);
    if (ret < 0) {
        goto err_delete_event_flag;
    }

    ret = ksceKernelStartThread(g_thid, 0, 0);
    if (ret < 0) {
        goto err_unregister;
    }

    hidkeyboard_driver_activated = 0;
    hidkeyboard_driver_registered = 1;

    return SCE_KERNEL_START_SUCCESS;

err_unregister:
    ksceUdcdUnregister(&g_driver);
err_delete_event_flag:
    ksceKernelDeleteEventFlag(usb_event_flag_id);
err_destroy_thread:
    ksceKernelDeleteThread(g_thid);
err_return:
    return SCE_KERNEL_START_FAILED;
}

int module_stop (SceSize args, void *argp)
{
    ksceKernelSetEventFlag(usb_event_flag_id, EVF_EXIT);

    if (g_thid > 0) {
        SceUInt timeout = 0xFFFFFFFF;
        g_run = 0;
        ksceKernelWaitThreadEnd(g_thid, NULL, &timeout);
        ksceKernelDeleteThread(g_thid);
    }

    ksceKernelDeleteEventFlag(usb_event_flag_id);

    ksceUdcdDeactivate();
    ksceUdcdStop(VITA_USB_KEYBOARD, 0, NULL);
    ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);
    ksceUdcdUnregister(&g_driver);

    return SCE_KERNEL_STOP_SUCCESS;
}

int hidkeyboard_user_start(void)
{
    int state = 0;
    int ret;

    ENTER_SYSCALL(state);

    if (!hidkeyboard_driver_registered) {
        EXIT_SYSCALL(state);
        return HIDKEYBOARD_ERROR_DRIVER_NOT_REGISTERED;
    }
    else if (hidkeyboard_driver_activated) {
        EXIT_SYSCALL(state);
        return HIDKEYBOARD_ERROR_DRIVER_ALREADY_ACTIVATED;
    }

    ret = ksceUdcdDeactivate();
    if (ret < 0 && ret != SCE_UDCD_ERROR_INVALID_ARGUMENT) {
        EXIT_SYSCALL(state);
        return ret;
    }

    ksceUdcdStop("USB_MTP_Driver", 0, NULL);
    ksceUdcdStop("USBPSPCommunicationDriver", 0, NULL);
    ksceUdcdStop("USBSerDriver", 0, NULL);
    ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);

    ret = ksceUdcdStart("USBDeviceControllerDriver", 0, 0);
    if (ret < 0) {
        EXIT_SYSCALL(state);
        return ret;
    }

    ret = ksceUdcdStart(VITA_USB_KEYBOARD, 0, 0);
    if (ret < 0) {
        ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);
        EXIT_SYSCALL(state);
        return ret;
    }

    ksceKernelClearEventFlag(usb_event_flag_id, ~EVF_ALL_MASK);

    ret = ksceUdcdActivate(VITA_USB_KEYBOARD_PID);
    if (ret < 0) {
        ksceUdcdStop(VITA_USB_KEYBOARD, 0, NULL);
        ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);
        EXIT_SYSCALL(state);
        return ret;
    }

    hidkeyboard_driver_activated = 1;

    EXIT_SYSCALL(state);
    return 0;
}

int hidkeyboard_user_stop(void)
{
    int state = 0;

    ENTER_SYSCALL(state);

    if (!hidkeyboard_driver_activated) {
        EXIT_SYSCALL(state);
        return HIDKEYBOARD_ERROR_DRIVER_NOT_ACTIVATED;
    }

    ksceKernelSetEventFlag(usb_event_flag_id, EVF_DISCONNECTED);

    ksceUdcdDeactivate();
    ksceUdcdStop(VITA_USB_KEYBOARD, 0, NULL);
    ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);
    ksceUdcdStart("USBDeviceControllerDriver", 0, NULL);
    ksceUdcdStart("USB_MTP_Driver", 0, NULL);
    ksceUdcdActivate(0x4E4);

    hidkeyboard_driver_activated = 0;

    EXIT_SYSCALL(state);
    return 0;
}

int HidKeyboardSendKey(void)
{
    int state = 0;

    ENTER_SYSCALL(state);

    log_reset();

    LOG("yay HidKeyboardSendKey works");

    log_flush();

    EXIT_SYSCALL(state);

    return 0;
}