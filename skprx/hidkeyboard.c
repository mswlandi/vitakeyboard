#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/ctrl.h>
#include <psp2kern/udcd.h>

#include "usb_descriptors.h"
#include "hidkeyboard.h"

#define Kprintf(...) (void)0

#define VITA_USB_KEYBOARD                    "VITA_KEYBOARD"
#define VITA_USB_KEYBOARD_PID                0x1338

static char g_inputs[8] __attribute__ ((aligned(64))) = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static struct SceUdcdDeviceRequest g_request;
static struct SceUdcdDeviceRequest g_reportrequest;
static SceUID g_thid = -1;
static int g_run = 1;


/* Forward define driver functions */
static int start_func (int size, void *args);
static int stop_func (int size, void *args);
static int usb_recvctl (int arg1, int arg2, struct SceUdcdEP0DeviceRequest *req);
static int usb_change (int interfaceNumber, int alternateSetting);
static int usb_attach (int usb_version);
static void usb_detach (void);
static void usb_configure (int usb_version, int desc_count, struct SceUdcdInterfaceSettings *settings);

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


static void send_inputs (void);

static
void complete_request (struct SceUdcdDeviceRequest *req)
{
    Kprintf ("complete_request\n");
    req->unused = NULL;
}


/* Device request */
static
int usb_recvctl (int arg1, int arg2, struct SceUdcdEP0DeviceRequest *req)
{
    Kprintf ("recvctl: %x %x\n", arg1, arg2);
    Kprintf ("request: %x type: %x wValue: %x wIndex: %x wLength: %x\n",
        req->bRequest, req->bmRequestType, req->wValue, req->wIndex, req->wLength);

    if (req->bmRequestType == 0x81 && req->bRequest == 0x06 && req->wValue == 0x2200 && arg2 != -1) {
        if (!g_reportrequest.unused) {
            g_reportrequest.data = hid_report;
            g_reportrequest.size = sizeof (hid_report);
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
int usb_change (int interfaceNumber, int alternateSetting)
{
    Kprintf ("usb_change %d %d\n", interfaceNumber, alternateSetting);
    return 0;
}

/* Attach callback */
static
int usb_attach (int usb_version)
{
    Kprintf ("usb_attach %d\n", usb_version);
    return 0;
}

/* Detach callback */
static
void usb_detach (void)
{
    Kprintf ("usb_detach\n");
}

static
void usb_configure (int usb_version, int desc_count, struct SceUdcdInterfaceSettings *settings)
{
    Kprintf ("usb_configure %d %d %p %d\n", usb_version, desc_count, settings, settings->numDescriptors);
}

/* USB start function */
static
int start_func (int size, void *p)
{
    Kprintf ("start\n");
    return 0;
}

/* USB stop function */
static
int stop_func (int size, void *p)
{
    Kprintf ("stop\n");
    return 0;
}

static
void send_inputs (void)
{
    if (!g_request.unused) {
        g_request.endpoint = &endpoints[1];
        g_request.data = g_inputs;
        g_request.size = sizeof (g_inputs);
        g_request.isControlRequest = 0;
        g_request.onComplete = &complete_request;
        g_request.transmitted = 0;
        g_request.returnCode = 0;
        g_request.unused = &g_request;
        g_request.next = NULL;
        g_request.physicalAddress = NULL;
        ksceUdcdReqSend (&g_request);
    }
}

static
int update_keyboard (SceSize args, void *argp)
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

        if (ksceUdcdGetDeviceState () & SCE_UDCD_STATUS_CONNECTION_ESTABLISHED && changed)
            send_inputs ();
        ksceKernelDelayThread (10000);

        changed = 0;
    }
    return 0;
}

/* Usb start routine*/
int keyboard_start (void)
{
    LOG("entered keyboard_start\n");

    int ret = 0;
    ret = ksceUdcdRegister (&g_driver);
    if (ret < 0) return ret;

    ksceUdcdDeactivate();
    ksceUdcdStop("USB_MTP_Driver", 0, NULL);
    ksceUdcdStop("USBPSPCommunicationDriver", 0, NULL);
    ksceUdcdStop("USBSerDriver", 0, NULL);
    ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);

    ret = ksceUdcdStart ("USBDeviceControllerDriver", 0, 0);
    if (ret < 0) return ret;
    ret = ksceUdcdStart (VITA_USB_KEYBOARD, 0, 0);
    if (ret < 0) return ret;

    ret = ksceUdcdActivate (VITA_USB_KEYBOARD_PID);
    if (ret < 0) return ret;

    g_thid = ksceKernelCreateThread ("update_thread", &update_keyboard, 0x3C, 0x1000, 0, 0x10000, 0);
    if (g_thid >= 0) ksceKernelStartThread (g_thid, 0, 0);
    else return g_thid;

    return 0;
}

/* Usb stop */
int keyboard_stop (void)
{
    if (g_thid > 0) {
        SceUInt timeout = 0xFFFFFFFF;
        g_run = 0;
        ksceKernelWaitThreadEnd(g_thid, NULL, &timeout);
        ksceKernelDeleteThread(g_thid);
    }

    ksceUdcdDeactivate ();
    ksceUdcdStop (VITA_USB_KEYBOARD, 0, 0);
    ksceUdcdStop ("USBDeviceControllerDriver", 0, 0);
    ksceUdcdUnregister (&g_driver);
    return 0;
}
