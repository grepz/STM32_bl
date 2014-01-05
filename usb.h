#ifndef __BL_USB_H
#define __BL_USB_H

static inline int usb_connect(void)
{
    return gpio_get(GPIOA, GPIO9);
}

void usb_gpio_init(void);
int usbd_create(void);
void usbd_stop(void);

void usb_msgsend(uint8_t *msg, size_t len);
int get_byte(uint8_t *buf, unsigned int timeout);
void buf_put(uint8_t b);
int buf_get(void);

#endif /* __BL_USB_H */
