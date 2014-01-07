#ifndef __BL_BOARD_H
#define __BL_BOARD_H

#define STM32F4_DISCOVERY

/* USART configuration */

#ifdef STM32F4_DISCOVERY
  #define BL_USART USART2
  #define BL_USART_RCC RCC_APB1ENR_USART2EN

  #define SERIAL_RX_PORT GPIOA
  #define SERIAL_TX_PORT GPIOA

  #define SERIAL_RX_PIN GPIO3
  #define SERIAL_TX_PIN GPIO2
#else
  #define BL_USART USART6
  #define BL_USART_RCC RCC_APB2ENR_USART6EN

  #define SERIAL_RX_PORT GPIOC
  #define SERIAL_TX_PORT GPIOC

  #define SERIAL_RX_PIN GPIO7
  #define SERIAL_TX_PIN GPIO6
#endif

/* Leds configuration */

#ifdef STM32F4_DISCOVERY
  #define BOARD_PORT_LEDS GPIOD

  #define BOARD_PIN_LED1 GPIO12
  #define BOARD_PIN_LED2 GPIO13
  #define BOARD_PIN_LED3 GPIO14
  #define BOARD_PIN_LED4 GPIO15
#else
  #define BOARD_PORT_LEDS GPIOD

  #define BOARD_PIN_LED1 GPIO13
  #define BOARD_PIN_LED2 GPIO14
  #define BOARD_PIN_LED3 GPIO13
  #define BOARD_PIN_LED4 GPIO14
#endif

/* USB configuration */


#endif /* __BL_BOARD_H */
