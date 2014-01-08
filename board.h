#ifndef __BL_BOARD_H
#define __BL_BOARD_H

//#define STM32F4_DISCOVERY

/* USART configuration */

#ifdef STM32F4_DISCOVERY
  #define BL_USART          USART2
  #define BL_USART_PORT_RCC RCC_AHB1ENR_IOPAEN

  #define SERIAL_RX_PORT GPIOA
  #define SERIAL_TX_PORT GPIOA

  #define SERIAL_RX_PIN GPIO3
  #define SERIAL_TX_PIN GPIO2

  #define USART_AF GPIO_AF7
#else
  #define BL_USART          USART6
  #define BL_USART_PORT_RCC RCC_AHB1ENR_IOPCEN

  #define SERIAL_RX_PORT GPIOC
  #define SERIAL_TX_PORT GPIOC

  #define SERIAL_RX_PIN GPIO7
  #define SERIAL_TX_PIN GPIO6

  #define USART_AF GPIO_AF8
#endif /* USART6 */

/* Leds configuration */

#ifdef STM32F4_DISCOVERY
  #define BL_LEDS_PORT GPIOD

  #define BL_LED1_PIN GPIO12
  #define BL_LED2_PIN GPIO13
  #define BL_LED3_PIN GPIO14
  #define BL_LED4_PIN GPIO15
#else
  #define BL_LEDS_PORT GPIOD

  #define BL_LED1_PIN GPIO13
  #define BL_LED2_PIN GPIO14
  #define BL_LED3_PIN GPIO13
  #define BL_LED4_PIN GPIO14
#endif /* Leds */

/* USB configuration */

#ifdef STM32F4_DISCOVERY
  #define BL_USB_FS_PORT GPIOA
  #define BL_USB_FS_PORT_RCC RCC_AHB1ENR_IOPAEN

  #define BL_USB_FS_DP_PIN   GPIO12
  #define BL_USB_FS_DM_PIN   GPIO11
  #define BL_USB_FS_ID_PIN   GPIO10
  #define BL_USB_FS_VBUS_PIN GPIO9
#else
  #define BL_USB_FS_PORT GPIOA
  #define BL_USB_FS_PORT_RCC RCC_AHB1ENR_IOPAEN

  #define BL_USB_FS_DP_PIN   GPIO12
  #define BL_USB_FS_DM_PIN   GPIO11
  #define BL_USB_FS_ID_PIN   GPIO10
  #define BL_USB_FS_VBUS_PIN GPIO9
#endif /* USB */

#endif /* __BL_BOARD_H */
