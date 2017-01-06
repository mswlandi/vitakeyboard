#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/ctrl.h>
#include <psp2kern/udcd.h>

#include "hidmouse.h"

#define Kprintf(...) (void)0

#define PSP_USB_MOUSE            "PSP_MOUSE"
#define PSP_USB_MOUSE_PID        0x7d

static char g_inputs[4] __attribute__ ((aligned(64))) = { 0x00, 0x00, 0x00, 0x00 };
static struct SceUdcdDeviceRequest g_request;
static struct SceUdcdDeviceRequest g_reportrequest;
static SceUID g_thid = -1;
static int g_run = 1;

static
unsigned char hid_report[] __attribute__ ((aligned(64))) = {
  0x05, 0x01,  /* Usage Page (Generic Desktop) */
  0x09, 0x02,  /* Usage (Mouse) */
  0xA1, 0x01,  /* Collection (Application) */
  0x09, 0x01,  /* Usage (Pointer) */
  0xA1, 0x00,  /* Collection (Physical) */
  0x05, 0x09,  /* Usage Page (Button) */
  0x19, 0x01,  /* Usage Minimum (Button 1) */
  0x29, 0x03,  /* Usage Maximum (Button 3) */
  0x15, 0x00,  /* Logical Minimum (0) */
  0x25, 0x01,  /* Logical Maximum (1) */
  0x95, 0x03,  /* Report Count (3) */
  0x75, 0x01,  /* Report Size (1) */
  0x81, 0x02,  /* Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit) */
  0x95, 0x01,  /* Report Count (1) */
  0x75, 0x05,  /* Report Size (5) */
  0x81, 0x01,  /* Input (Cnst,Ary,Abs) */
  0x05, 0x01,  /* Usage Page (Generic Desktop) */
  0x09, 0x30,  /* Usage (X) */
  0x09, 0x31,  /* Usage (Y) */
  0x09, 0x38,  /* Usage (Wheel) */
  0x15, 0x81,  /* Logical Minimum (-127) */
  0x25, 0x7F,  /* Logical Maximum (127) */
  0x75, 0x08,  /* Report Size (8) */
  0x95, 0x03,  /* Report Count (3) */
  0x81, 0x06,  /* Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit) */
  0xC0,        /* End Collection */
  0xC0         /* End Collection */
} ;


/* HID descriptor */
static
unsigned char hiddesc[] = {
  0x09, /* bLength */
  0x21, /* bDescriptorType */
  0x10, 0x01, /* bcdHID */
  0x00, /* bCountryCode */
  0x01, /* bNumDescriptors */
  0x22, /* bDescriptorType */
  sizeof (hid_report), 0x00 /* wDescriptorLength */
};

/* Endpoint blocks */
static
struct SceUdcdEndpoint endpoints[2] = {
  { 0x00, 0, 0, 0 },
  { 0x80, 1, 0, 0 }
};

/* Interfaces */
static
struct SceUdcdInterface interfaces[1] = {
  { -1, 0, 1 }
};

/* String descriptor */
static
struct SceUdcdStringDescriptor descriptors[2] = {
  {
    16,
    USB_DT_STRING,
    { 'M', 'o', 'u', 's', 'e',  ' ', '1' }
  },
  {
    0,
    USB_DT_STRING
  }
};

/* HI-Speed device descriptor */
static
struct SceUdcdDeviceDescriptor devdesc_hi =
{
  USB_DT_DEVICE_SIZE,
  USB_DT_DEVICE,
  0x200,         /* bcdUSB */
  USB_CLASS_PER_INTERFACE,  /* bDeviceClass */
  0,             /* bDeviceSubClass */
  0,             /* bDeviceProtocol */
  64,            /* bMaxPacketSize0 */
  0,             /* idProduct */
  0,             /* idVendor */
  0x100,         /* bcdDevice */
  0,             /* iManufacturer */
  0,             /* iProduct */
  0,             /* iSerialNumber */
  1              /* bNumConfigurations */
};

/* Hi-Speed endpoint descriptors */
static
struct SceUdcdEndpointDescriptor endpdesc_hi[2] =
{
  {
    USB_DT_ENDPOINT_SIZE ,
    USB_DT_ENDPOINT,
    0x81, /* bEndpointAddress */
    0x03, /* bmAttributes */
    0x04, /* wMaxPacketSize */
    0x05  /* bInterval */
  },
  {
    0,
  }
};

/* Hi-Speed interface descriptor */
static
struct SceUdcdInterfaceDescriptor interdesc_hi[2] =
{
  {
    USB_DT_INTERFACE_SIZE,
    USB_DT_INTERFACE,
    0,      /* bInterfaceNumber */
    0,      /* bAlternateSetting */
    1,      /* bNumEndpoints */
    USB_CLASS_HID ,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* bInterfaceProtocol */
    1,      /* iInterface */
    &endpdesc_hi[0], /* endpoints */
    hiddesc,
    sizeof (hiddesc)
  },
  {
    0
  }
};

/* Hi-Speed settings */
static
struct SceUdcdInterfaceSettings settings_hi[1] =
{
  {
    &interdesc_hi[0],
    0,
    1
  }
};

/* Hi-Speed configuration descriptor */
static
struct SceUdcdConfigDescriptor confdesc_hi =
{
  USB_DT_CONFIG_SIZE,
  USB_DT_CONFIG,
  (USB_DT_INTERFACE_SIZE + USB_DT_CONFIG_SIZE + USB_DT_ENDPOINT_SIZE + sizeof (hiddesc)), /* wTotalLength */
  1,      /* bNumInterfaces */
  1,      /* bConfigurationValue */
  0,      /* iConfiguration */
  0xC0,   /* bmAttributes */
  0,      /* bMaxPower */
  &settings_hi[0]
};


/* Hi-Speed configuration */
static
struct SceUdcdConfiguration config_hi =
{
  &confdesc_hi,
  &settings_hi[0],
  &interdesc_hi[0],
  &endpdesc_hi[0]
};

/* Full-Speed device descriptor */
static
struct SceUdcdDeviceDescriptor devdesc_full =
{
  USB_DT_DEVICE_SIZE,
  USB_DT_DEVICE,
  0x200,         /* bcdUSB (should be 0x110 but the PSVita freezes otherwise) */
  USB_CLASS_PER_INTERFACE,  /* bDeviceClass */
  0,             /* bDeviceSubClass */
  0,             /* bDeviceProtocol */
  8,             /* bMaxPacketSize0 */
  0,             /* idProduct */
  0,             /* idVendor */
  0x200,         /* bcdDevice */
  0,             /* iManufacturer */
  0,             /* iProduct */
  0,             /* iSerialNumber */
  1              /* bNumConfigurations */
};

/* Full-Speed endpoint descriptors */
static
struct SceUdcdEndpointDescriptor endpdesc_full[2] =
{
  {
    USB_DT_ENDPOINT_SIZE ,
    USB_DT_ENDPOINT,
    0x81, /* bEndpointAddress */
    0x03, /* bmAttributes */
    0x04, /* wMaxPacketSize */
    0x05  /* bInterval */
  },
  {
    0,
  }
};


/* Full-Speed interface descriptor */
static
struct SceUdcdInterfaceDescriptor interdesc_full[2] =
{
  {
    USB_DT_INTERFACE_SIZE,
    USB_DT_INTERFACE,
    0,      /* bInterfaceNumber */
    0,      /* bAlternateSetting */
    1,      /* bNumEndpoints */
    USB_CLASS_HID,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* bInterfaceProtocol */
    1,      /* iInterface */
    &endpdesc_full[0], /* endpoints */
    hiddesc,
    sizeof (hiddesc)
  },
  {
    0
  }
};


/* Full-Speed settings */
static
struct SceUdcdInterfaceSettings settings_full[1] =
{
  {
    &interdesc_full[0],
    0,
    1
  }
};

/* Full-Speed configuration descriptor */
static
struct SceUdcdConfigDescriptor confdesc_full =
{
  USB_DT_CONFIG_SIZE,
  USB_DT_CONFIG,
  (USB_DT_INTERFACE_SIZE + USB_DT_CONFIG_SIZE + USB_DT_ENDPOINT_SIZE + sizeof (hiddesc)), /* wTotalLength */
  1,      /* bNumInterfaces */
  1,      /* bConfigurationValue */
  0,      /* iConfiguration */
  0xC0,   /* bmAttributes */
  0,      /* bMaxPower */
  &settings_full[0]
};

/* Full-Speed configuration */
static
struct SceUdcdConfiguration config_full =
{
  &confdesc_full,
  &settings_full[0],
  &interdesc_full[0],
  &endpdesc_full[0]
};


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
  PSP_USB_MOUSE,               /* driverName */
  2,                           /* numEndpoints */
  &endpoints[0],               /* endpoints */
  &interfaces[0],              /* interface */
  &devdesc_hi,                 /* descriptor_hi */
  &config_hi,                  /* configuration_hi */
  &devdesc_full,               /* descriptor */
  &config_full,                /* configuration */
  &descriptors[0],             /* stringDescriptors */
  NULL,
  NULL,
  &usb_recvctl,                /* processRequest */
  &usb_change,                 /* chageSetting */
  &usb_attach,                 /* attach */
  &usb_detach,                 /* detach */
  &usb_configure,              /* configure */
  &start_func,                 /* start func */
  &stop_func,                  /* stop func */
  0,
  0,
  NULL                         /* link to driver */
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
      ksceUdcdReqSend (&g_reportrequest);
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
int update_mouse (SceSize args, void *argp)
{
  SceCtrlData pad;
  unsigned char x0 = 0, y0 = 0;
  int first = 1;

  //ksceCtrlSetSamplingCycle (0);
  ksceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
  while (g_run) {
    if (ksceCtrlReadBufferPositive (0, &pad, 1) >= 0) {
      g_inputs[1] = (pad.lx >> 3) - x0;
      g_inputs[2] = (pad.ly >> 3) - y0;
      g_inputs[0] = g_inputs[3] = 0;
      if (pad.buttons & SCE_CTRL_UP)
        g_inputs[3] = +1;
      if (pad.buttons & SCE_CTRL_DOWN)
        g_inputs[3] = -1;
      if (pad.buttons & SCE_CTRL_CROSS)
        g_inputs[0] |= 1;
      if (pad.buttons & SCE_CTRL_CIRCLE)
        g_inputs[0] |= 2;
      if (pad.buttons & SCE_CTRL_SQUARE)
        g_inputs[0] |= 4;
      ksceKernelCpuDcacheWritebackRange (g_inputs, sizeof (g_inputs));

    }
    if (first) {
      x0 = g_inputs[1];
      y0 = g_inputs[2];
    }
    if (ksceUdcdGetDeviceState () & SCE_UDCD_STATUS_CONNECTION_ESTABLISHED)
      send_inputs ();
    ksceKernelDelayThread (20000);
    first = 0;
  }
  return 0;
}

/* Usb start routine*/
int mouse_start (void)
{
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
  ret = ksceUdcdStart (PSP_USB_MOUSE, 0, 0);
  if (ret < 0) return ret;

  ret = ksceUdcdActivate (PSP_USB_MOUSE_PID);
  if (ret < 0) return ret;

  g_thid = ksceKernelCreateThread ("update_thread", &update_mouse, 0x3C, 0x1000, 0, 0x10000, 0);
  if (g_thid >= 0) ksceKernelStartThread (g_thid, 0, 0);
  else return g_thid;

  return 0;
}

/* Usb stop */
int mouse_stop (void)
{
  if (g_thid > 0) {
    SceUInt timeout = 0xFFFFFFFF;
    g_run = 0;
    ksceKernelWaitThreadEnd(g_thid, NULL, &timeout);
    ksceKernelDeleteThread(g_thid);
  }

  ksceUdcdDeactivate ();
  ksceUdcdStop (PSP_USB_MOUSE, 0, 0);
  ksceUdcdStop ("USBDeviceControllerDriver", 0, 0);
  ksceUdcdUnregister (&g_driver);
  return 0;
}
