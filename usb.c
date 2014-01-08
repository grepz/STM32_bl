#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#if 0
#include <libopencm3/stm32/f4/exti.h>
#endif
#include <libopencm3/stm32/f4/nvic.h>

#include "defs.h"

#include "usb.h"
#include "timer.h"
#include "led.h"

static void __cdcacm_set_config(usbd_device *dev, uint16_t wValue);
static int __cdcacm_control_request(
    usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req));
static void __cdcacm_data_rx_cb(usbd_device *dev, uint8_t ep);

//static uint8_t vbus_state = 0;

typedef struct cdcacm_func_descr {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_func_descr_t;

#define USBD_BUF_LEN 512
#define USB_RX_LEN   512

static uint8_t __usbd_buf[USBD_BUF_LEN];
static usbd_device *usbd;
static unsigned head = 0, tail = 0;
static uint8_t __usb_rx[USB_RX_LEN];
static const char *__usb_strings[] = {
    "SapTech",
    "TR",
    "CTRL",
};

static const cdcacm_func_descr_t __cdc_fdescr = {
    .header = {
        .bFunctionLength    = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType    = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
        .bcdCDC             = 0x0110,
    },
    .call_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
        .bDescriptorType    = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
        .bmCapabilities     = 0,
        .bDataInterface     = 1,
    },
    .acm = {
        .bFunctionLength    = sizeof(struct usb_cdc_acm_descriptor),
        .bDescriptorType    = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_ACM,
        .bmCapabilities     = 0,
    },
    .cdc_union = {
        .bFunctionLength        = sizeof(struct usb_cdc_union_descriptor),
        .bDescriptorType        = CS_INTERFACE,
        .bDescriptorSubtype     = USB_CDC_TYPE_UNION,
        .bControlInterface      = 0,
        .bSubordinateInterface0 = 1,
    }
};

static const struct usb_endpoint_descriptor __comm_endp[] = {
    {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize   = 16,
	.bInterval        = 255,
    }
};

static const struct usb_endpoint_descriptor __data_endp[] = {
    {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes     = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize   = 64,
	.bInterval        = 1,
    },
    {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes     = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize   = 64,
	.bInterval        = 1,
    }
};

static const struct usb_interface_descriptor __comm_iface[] = {
    {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = 0,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 1,
	.bInterfaceClass    = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface         = 0,
	.endpoint           = __comm_endp,
	.extra              = &__cdc_fdescr,
	.extralen           = sizeof(__cdc_fdescr)
    }
};

static const struct usb_interface_descriptor __data_iface[] = {
    {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = 1,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 2,
	.bInterfaceClass    = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface         = 0,
	.endpoint           = __data_endp,
    }
};

static const struct usb_interface __usb_ifaces[] = {
    {
	.num_altsetting = 1,
	.altsetting     = __comm_iface,
    },
    {
	.num_altsetting = 1,
	.altsetting     = __data_iface,
    }
};

static const struct usb_device_descriptor __usbdev_desc = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = USB_CLASS_CDC,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x26AC,
    .idProduct          = 0x0001,
    .bcdDevice          = 0x0101,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1,
};

static const struct usb_config_descriptor __usbconf_desc = {
    .bLength             = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType     = USB_DT_CONFIGURATION,
    .wTotalLength        = 0,
    .bNumInterfaces      = 2,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0x80,
    .bMaxPower           = 0xFA, /* 500 mA, 250 get doubled */
    .interface           = __usb_ifaces,
};

#if 0
void usb_gpio_init(void)
{
    /* GPIO9 to sniff VBUS */
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO9);

    rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
                    GPIO9 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);
}
#endif

void usb_gpio_init(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO9);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10|GPIO11|GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10|GPIO11|GPIO12);
}

int usbd_create(void)
{
    usbd = usbd_init(&otgfs_usb_driver, &__usbdev_desc, &__usbconf_desc,
                     __usb_strings, 3, __usbd_buf, sizeof(__usbd_buf));
    if (!usbd)
        return -1;

    usbd_register_set_config_callback(usbd, __cdcacm_set_config);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);

    return 0;
}

void usbd_stop(void)
{
    usbd_disconnect(usbd, true);
    nvic_disable_irq(NVIC_OTG_FS_IRQ);
}

void buf_put(uint8_t b)
{
    unsigned next = (head + 1) % sizeof(__usb_rx);

    if (next != tail) {
        __usb_rx[head] = b;
        head = next;
    }
}

int buf_get(void)
{
    int	ret = -1;

    if (tail != head) {
        ret = __usb_rx[tail];
        tail = (tail + 1) % sizeof(__usb_rx);
    }

    return ret;
}

void usb_msgsend(uint8_t *buf, size_t len)
{
    while (len) {
        unsigned int sz = (len > 256) ? 256 : len;
        unsigned int sent;

        sent = usbd_ep_write_packet(usbd, 0x82, buf, sz);

        len -= sent;
        buf += sent;
    }
}

int get_byte(uint8_t *buf, unsigned int timeout)
{
    int c = -1;

    /* start the timeout */
    set_timer(TIMER_IO, timeout);

    do {
        c = buf_get();
        if (c >= 0) {
            *buf = (uint8_t)c;
            break;
        }
    } while (check_timer(TIMER_IO) > 0);

    return c;
}

void otg_fs_isr(void)
{
    usbd_poll(usbd);
}

/* ======================================= */
/* Local functions                         */
/* ======================================= */

static void __cdcacm_set_config(usbd_device *dev, uint16_t wValue)
{
    (void)dev;
    (void)wValue;

    usbd_ep_setup(dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, __cdcacm_data_rx_cb);
    usbd_ep_setup(dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(
        dev,  USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        __cdcacm_control_request);
}

static int __cdcacm_control_request(
    usbd_device *dev, struct usb_setup_data *req,uint8_t **buf,uint16_t *len,
    void (**complete)(usbd_device *dev, struct usb_setup_data *req))
{
    /* + */
    (void)complete;
    (void)buf;
    (void)dev;

    switch (req->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
        /*
         * This Linux cdc_acm driver requires this to be implemented
         * even though it's optional in the CDC spec, and we don't
         * advertise it in the ACM functional descriptor.
         */
        return 1;
    }
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding))
            return 0;

        return 1;
    }

    return 0;
}

static void __cdcacm_data_rx_cb(usbd_device *dev, uint8_t ep)
{
    (void)ep;
    char buf[64];
    int i, len = usbd_ep_read_packet(dev, 0x01, buf, 64);
    for (i = 0; i < len; i++)
        buf_put(buf[i]);
}
