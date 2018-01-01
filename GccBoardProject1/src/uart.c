/*
 * uart.c
 *
 * Created: 12/31/2017 11:27:06 AM
 *  Author: Sebastian
 */ 
 #include "asf.h"

 void uart_init(void)
 {
    const sam_usart_opt_t usart_console_settings = {
	    USART_SERIAL_BAUDRATE,
	    USART_SERIAL_CHAR_LENGTH,
	    USART_SERIAL_PARITY,
	    USART_SERIAL_STOP_BIT,
	    US_MR_CHMODE_NORMAL
    };

	//#if SAM4L
	//sysclk_enable_peripheral_clock(USART_SERIAL);
	//#else
	//sysclk_enable_peripheral_clock(USART_SERIAL_ID);
	//#endif
	//usart_init_rs232(USART_SERIAL, &usart_console_settings,
	//sysclk_get_main_hz());
	//usart_enable_tx(USART_SERIAL);
	//usart_enable_rx(USART_SERIAL);

	/* Configure console UART. */
	stdio_serial_init(USART_SERIAL, &usart_console_settings);
 }