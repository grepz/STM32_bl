#ifndef __BL_BOARD_H
#define __BL_BOARD_H

/* USART configuration */

#define BL_USART          USART6
#define BL_USART_PORT_RCC RCC_AHB1ENR_IOPCEN

#define SERIAL_RX_PORT GPIOC
#define SERIAL_TX_PORT GPIOC

#define SERIAL_RX_PIN GPIO7
#define SERIAL_TX_PIN GPIO6

#define USART_AF GPIO_AF8

/* Leds configuration */
#define BL_LEDS_PORT GPIOD

#define BL_LED1_PIN GPIO13
#define BL_LED2_PIN GPIO14
#define BL_LED3_PIN GPIO13
#define BL_LED4_PIN GPIO14

/* USB configuration */

#define BL_USB_FS_PORT GPIOA
#define BL_USB_FS_PORT_RCC RCC_AHB1ENR_IOPAEN

#define BL_USB_FS_DP_PIN   GPIO12
#define BL_USB_FS_DM_PIN   GPIO11
#define BL_USB_FS_ID_PIN   GPIO10
#define BL_USB_FS_VBUS_PIN GPIO9

/* SPI */
#define BL_SPI2_PORT GPIOB

#define BL_SPI2_NSS  GPIO12
#define BL_SPI2_SCK  GPIO13
#define BL_SPI2_MISO GPIO14
#define BL_SPI2_MOSI GPIO15

#define BL_SPI2_RST GPIO10
#define BL_SPI2_WP  GPIO11


#endif /* __BL_BOARD_H */
