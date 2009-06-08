#include <string.h>
#include <pspkernel.h>
#include <pspkdebug.h>

#include "hidmouse.h"
#include "usb.h"

#define PSP_USB_MOUSE            "pspmouse"
#define PSP_USB_MOUSE_PID        0x7d

static char g_inputs[4] __attribute__ ((aligned(64))) = { 0x00, 0x01, 0x01, 0x00 };
static struct UsbbdDeviceRequest g_request;
static struct UsbbdDeviceRequest g_reportrequest;
static SceUID g_mainalarm = -1;

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
struct UsbEndpoint endpoints[2] = {
  { 0, 0, 0 },
  { 1, 0, 0 }
};

/* Interfaces */
static
struct UsbInterface interfaces[1] = {
  { -1, 0, 1 }
};

/* String descriptor */
static
struct StringDescriptor descriptors[2] = {
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
struct DeviceDescriptor devdesc_hi =
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
struct EndpointDescriptor endpdesc_hi[2] =
{
  {
    USB_DT_ENDPOINT_SIZE ,
    USB_DT_ENDPOINT,
    0x81, /* bEndpointAddress */
    0x03, /* bmAttributes */
    0x04, /* wMaxPacketSize */
    0x0A  /* bInterval */
  },
  {
    0,
  }
};

/* Hi-Speed interface descriptor */
static
struct InterfaceDescriptor interdesc_hi[2] =
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
struct InterfaceSettings settings_hi[1] =
{
  {
    &interdesc_hi[0],
    0,
    1
  }
};

/* Hi-Speed configuration descriptor */
static
struct ConfigDescriptor confdesc_hi =
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
struct UsbConfiguration config_hi =
{
  &confdesc_hi,
  &settings_hi[0],
  &interdesc_hi[0],
  &endpdesc_hi[0]
};

/* Full-Speed device descriptor */
static
struct DeviceDescriptor devdesc_full =
{
  USB_DT_DEVICE_SIZE,
  USB_DT_DEVICE,
  0x110,         /* bcdUSB */
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
struct EndpointDescriptor endpdesc_full[2] =
{
  {
    USB_DT_ENDPOINT_SIZE ,
    USB_DT_ENDPOINT,
    0x81, /* bEndpointAddress */
    0x03, /* bmAttributes */
    0x04, /* wMaxPacketSize */
    0x0A  /* bInterval */
  },
  {
    0,
  }
};


/* Full-Speed interface descriptor */
static
struct InterfaceDescriptor interdesc_full[2] =
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
struct InterfaceSettings settings_full[1] =
{
  {
    &interdesc_full[0],
    0,
    1
  }
};

/* Full-Speed configuration descriptor */
static
struct ConfigDescriptor confdesc_full =
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
struct UsbConfiguration config_full =
{
  &confdesc_full,
  &settings_full[0],
  &interdesc_full[0],
  &endpdesc_full[0]
};


/* Forward define driver functions */
static int start_func (int size, void *args);
static int stop_func (int size, void *args);
static int usb_recvctl (int arg1, int arg2, struct DeviceRequest *req);
static int usb_change (int interfaceNumber, int alternateSetting);
static int usb_attach (int usb_version);
static void usb_detach (void);
static void usb_configure (int usb_version, int desc_count, struct InterfaceSettings *settings);

/* USB host driver */
struct UsbDriver g_driver =
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
  &usb_recvctl,                /* processRequest */
  &usb_change,                 /* chageSetting */
  &usb_attach,                 /* attach */
  &usb_detach,                 /* detach */
  &usb_configure,              /* configure */
  &start_func,                 /* start func */
  &stop_func,                  /* stop func */
  NULL                         /* link to driver */
};


static void send_inputs (void);

static
SceUInt alarm_handler (void *common)
{
  send_inputs ();
  return 10000;
}

static
void complete_request (struct UsbbdDeviceRequest *req)
{
  Kprintf ("complete_request\n");
  req->unused = NULL;
}


/* Device request */
static
int usb_recvctl (int arg1, int arg2, struct DeviceRequest *req)
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
      sceUsbbdReqSend (&g_reportrequest);
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
  if (g_mainalarm < 0) g_mainalarm = sceKernelSetAlarm (10000, &alarm_handler, NULL);
  return 0;
}

/* Detach callback */
static
void usb_detach (void)
{
  Kprintf ("usb_detach\n");
  if (g_mainalarm >= 0) sceKernelCancelAlarm (g_mainalarm);
  g_mainalarm = -1;
}

static
void usb_configure (int usb_version, int desc_count, struct InterfaceSettings *settings)
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
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);
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
    sceUsbbdReqSend (&g_request);
  }
}

/* Usb start routine*/
int mouse_start (void)
{
  int ret = 0;
  ret = sceUsbbdRegister (&g_driver);
  if (ret < 0) return ret;
  ret = sceUsbStart (PSP_USB_BUS_DRIVERNAME, 0, 0);
  if (ret < 0) return ret;
  ret = sceUsbStart (PSP_USB_MOUSE, 0, 0);
  if (ret < 0) return ret;

  ret = sceUsbActivate (PSP_USB_MOUSE_PID);
  if (ret < 0) return ret;
  
  sceCtrlSetSamplingCycle (0);
  sceCtrlSetSamplingMode (PSP_CTRL_MODE_ANALOG);
  
  return ret;
}

/* Usb stop */
int mouse_stop (void)
{
  sceUsbDeactivate ();
  sceUsbStop (PSP_USB_MOUSE, 0, 0);
  sceUsbStop (PSP_USB_BUS_DRIVERNAME, 0, 0);
  sceUsbbdUnregister (&g_driver);
  return 0;
}
