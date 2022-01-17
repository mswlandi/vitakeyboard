// The descriptors are xerpi/hidmouse's descriptors modified according to
// https://www.usb.org/sites/default/files/documents/hid1_11.pdf
// (page 76 - Appendix E: Example USB Descriptors for HID Class Devices)

static
unsigned char hid_report[] __attribute__((aligned(64))) = {
    0x05, 0x01, // Usage Page(Generic Desktop),
    0x09, 0x06, // Usage(Keyboard),
    0xA1, 0x01, // Collection(Application),

    0x05, 0x07, //  Usage Page(Key Codes);
    0x19, 0xE0, //  Usage Minimum(224), [left control]
    0x29, 0xE7, //  Usage Maximum(231), [right GUI]
    0x15, 0x00, //  Logical Minimum(0),
    0x25, 0x01, //  Logical Maximum(1),
    0x75, 0x01, //  Report Size(1),
    0x95, 0x08, //  Report Count(8),
    0x81, 0x02, //  Input(Data, Variable, Absolute), [Modifier byte]

    0x95, 0x01, //  Report Count(1),
    0x75, 0x08, //  Report Size(8),
    0x81, 0x01, //  Input(Constant), [Reserved byte]

    0x95, 0x05, //  Report Count(5),
    0x75, 0x01, //  Report Size(1),
    0x05, 0x08, //  Usage Page (Page# for LEDs),
    0x19, 0x01, //  Usage Minimum(1),
    0x29, 0x05, //  Usage Maximum(5),
    0x91, 0x02, //  Output(Data, Variable, Absolute), [LED report]

    0x95, 0x01, //  Report Count(1),
    0x75, 0x03, //  Report Size(3),
    0x91, 0x01, //  Output(Constant), [LED report padding]

    0x95, 0x06, //  Report Count(6),
    0x75, 0x08, //  Report Size(8),
    0x15, 0x00, //  Logical Minimum(0),
    0x25, 0x65, //  Logical Maximum(101),
    0x05, 0x07, //  Usage Page(Key Codes),
    0x19, 0x00, //  Usage Minimum(0),
    0x29, 0x65, //  Usage Maximum(101),
    0x81, 0x00, //  Input(Data, Array), [Key arrays(6 bytes)]
    0xC0        // End Collection
};

/* HID descriptor */
static
unsigned char hiddesc[] = {
    0x09, /* bLength */
    0x21, /* bDescriptorType */
    0x11, 0x01, /* bcdHID */
    0x00, /* bCountryCode */
    0x01, /* bNumDescriptors */
    0x22, /* bDescriptorType */
    sizeof(hid_report), 0x00 /* wDescriptorLength */
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
        15,
        USB_DT_STRING,
        { 'v', 'i', 't', 'a', '-',  'k', 'e', 'y', 'b', 'o', 'a', 'r', 'd' }
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
        0x40, /* wMaxPacketSize */
        0x01  /* bInterval */
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
        0,                /* bInterfaceNumber */
        0,                /* bAlternateSetting */
        1,                /* bNumEndpoints */
        USB_CLASS_HID ,   /* bInterfaceClass */
        0x00,             /* bInterfaceSubClass */  // boot interface subclass
        0x00,             /* bInterfaceProtocol */  // keyboard
        1,                /* iInterface */
        &endpdesc_hi[0],  /* endpoints */
        hiddesc,
        sizeof(hiddesc)
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
    (USB_DT_INTERFACE_SIZE + USB_DT_CONFIG_SIZE + USB_DT_ENDPOINT_SIZE + sizeof(hiddesc)), /* wTotalLength */
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
    0x200,                    /* bcdUSB (should be 0x110 but the PSVita freezes otherwise) */
    USB_CLASS_PER_INTERFACE,  /* bDeviceClass */
    0,                        /* bDeviceSubClass */
    0,                        /* bDeviceProtocol */
    8,                        /* bMaxPacketSize0 */
    0,                        /* idProduct */
    0,                        /* idVendor */
    0x200,                    /* bcdDevice */
    0,                        /* iManufacturer */
    0,                        /* iProduct */
    0,                        /* iSerialNumber */
    1                         /* bNumConfigurations */
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
        0x40, /* wMaxPacketSize */
        0x01  /* bInterval */
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
        0,                  /* bInterfaceNumber */
        0,                  /* bAlternateSetting */
        1,                  /* bNumEndpoints */
        USB_CLASS_HID,      /* bInterfaceClass */
        0x00,               /* bInterfaceSubClass */  // boot interface subclass
        0x00,               /* bInterfaceProtocol */  // keyboard
        1,                  /* iInterface */
        &endpdesc_full[0],  /* endpoints */
        hiddesc,
        sizeof(hiddesc)
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
    (USB_DT_INTERFACE_SIZE + USB_DT_CONFIG_SIZE + USB_DT_ENDPOINT_SIZE + sizeof(hiddesc)), /* wTotalLength */
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