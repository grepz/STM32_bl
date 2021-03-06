#ifndef __BL_USB_H
#define __BL_USB_H

#define USB_EP_OUT 0x01
#define USB_EP_IN  0x82
#define USB_EP_CMD 0x83

static inline int usb_connect(void)
{
    return gpio_get(GPIOA, GPIO9);
}

void usb_gpio_init(void);
int usbd_start(void);
void usbd_stop(void);

void usb_msgsend(uint8_t *msg, size_t len);
int get_byte(uint8_t *buf, unsigned int timeout);
void buf_put(uint8_t b);
int buf_get(void);

#endif /* __BL_USB_H */
