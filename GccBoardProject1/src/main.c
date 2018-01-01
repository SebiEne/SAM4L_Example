/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

#define TC_CAPTURE_TIMER_SELECTION TC_CMR_TCCLKS_TIMER_CLOCK3


struct waveconfig_t {
	/** Internal clock signals selection. */
	uint32_t ul_intclock;
	/** Waveform frequency (in Hz). */
	uint16_t us_frequency;
	/** Duty cycle in percent (positive).*/
	uint16_t us_dutycycle;
};

/** TC waveform configurations */
static const struct waveconfig_t gc_waveconfig[] = {
	{TC_CMR_TCCLKS_TIMER_CLOCK4, 178, 30},
	{TC_CMR_TCCLKS_TIMER_CLOCK3, 375, 50},
	{TC_CMR_TCCLKS_TIMER_CLOCK3, 800, 75},
	{TC_CMR_TCCLKS_TIMER_CLOCK2, 1000, 80},
	{TC_CMR_TCCLKS_TIMER_CLOCK2, 4000, 55}
};


#if (SAM4L)
/* The first one is meaningless */
static const uint32_t divisors[5] = { 0, 2, 8, 32, 128};
#else
/* The last one is meaningless */
static const uint32_t divisors[5] = { 2, 8, 32, 128, 0};
#endif

/** Current wave configuration*/
static uint8_t gs_uc_configuration = 0;

/** Number of available wave configurations */
const uint8_t gc_uc_nbconfig = sizeof(gc_waveconfig)
/ sizeof(struct waveconfig_t);

/** Capture status*/
static uint32_t gs_ul_captured_pulses;
static uint32_t gs_ul_captured_ra;
static uint32_t gs_ul_captured_rb;



static void tc_waveform_initialize(void)
{
	uint32_t ra, rc;

	/* Configure the PMC to enable the TC module. */
	sysclk_enable_peripheral_clock(ID_TC_WAVEFORM);

	/* Init TC to waveform mode. */
	tc_init(TC, TC_CHANNEL_WAVEFORM,
	/* Waveform Clock Selection */
	gc_waveconfig[gs_uc_configuration].ul_intclock
	| TC_CMR_WAVE /* Waveform mode is enabled */
	| TC_CMR_ACPA_SET /* RA Compare Effect: set */
	| TC_CMR_ACPC_CLEAR /* RC Compare Effect: clear */
	| TC_CMR_CPCTRG /* UP mode with automatic trigger on RC Compare */
	);

	/* Configure waveform frequency and duty cycle. */
	rc = (sysclk_get_peripheral_bus_hz(TC) /
	divisors[gc_waveconfig[gs_uc_configuration].ul_intclock]) /
	gc_waveconfig[gs_uc_configuration].us_frequency;
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
	ra = (100 - gc_waveconfig[gs_uc_configuration].us_dutycycle) * rc / 100;
	tc_write_ra(TC, TC_CHANNEL_WAVEFORM, ra);

	/* Enable TC TC_CHANNEL_WAVEFORM. */
	tc_start(TC, TC_CHANNEL_WAVEFORM);
	printf("Start waveform: Frequency = %d Hz,Duty Cycle = %2d%%\n\r",
	gc_waveconfig[gs_uc_configuration].us_frequency,
	gc_waveconfig[gs_uc_configuration].us_dutycycle);
}

static void tc_capture_initialize(void)
{
	/* Configure the PMC to enable the TC module */
	sysclk_enable_peripheral_clock(ID_TC_CAPTURE);

	/* Init TC to capture mode. */
	tc_init(TC, TC_CHANNEL_CAPTURE,
	TC_CAPTURE_TIMER_SELECTION /* Clock Selection */
	| TC_CMR_LDRA_RISING /* RA Loading: rising edge of TIOA */
	| TC_CMR_LDRB_FALLING /* RB Loading: falling edge of TIOA */
	| TC_CMR_ABETRG /* External Trigger: TIOA */
	| TC_CMR_ETRGEDG_FALLING /* External Trigger Edge: Falling edge */
	);
}
void TC_Handler(void)
{
	if ((tc_get_status(TC, TC_CHANNEL_CAPTURE) & TC_SR_LDRBS) == TC_SR_LDRBS) {
		gs_ul_captured_pulses++;
		gs_ul_captured_ra = tc_read_ra(TC, TC_CHANNEL_CAPTURE);
		gs_ul_captured_rb = tc_read_rb(TC, TC_CHANNEL_CAPTURE);
	}
}

int main (void)
{
	sysclk_init();
	board_init();
	uart_init();

	/* Output example information */
	printf("-- TC capture waveform Example --\r\n");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

	//! [tc_waveform_gpio]
	/** Configure PIO Pins for TC */
	ioport_set_pin_mode(PIN_TC_WAVEFORM, PIN_TC_WAVEFORM_MUX);
	/** Disable I/O to enable peripheral mode) */
	ioport_disable_pin(PIN_TC_WAVEFORM);
	//! [tc_waveform_gpio]

	//! [tc_capture_gpio]
	/** Configure PIO Pins for TC */
	ioport_set_pin_mode(PIN_TC_CAPTURE, PIN_TC_CAPTURE_MUX);
	/** Disable I/O to enable peripheral mode) */
	ioport_disable_pin(PIN_TC_CAPTURE);
	//! [tc_capture_gpio]

	/* Configure TC TC_CHANNEL_WAVEFORM as waveform operating mode */
	printf("Configure TC%d channel %d as waveform operating mode \n\r",
	TC_PERIPHERAL, TC_CHANNEL_WAVEFORM);
	//! [tc_waveform_init_call]
	tc_waveform_initialize();
	//! [tc_waveform_init_call]

	/* Configure TC TC_CHANNEL_CAPTURE as capture operating mode */
	printf("Configure TC%d channel %d as capture operating mode \n\r",
	TC_PERIPHERAL, TC_CHANNEL_CAPTURE);
	//! [tc_capture_init_call]
	tc_capture_initialize();
	//! [tc_capture_init_call]

	//! [tc_capture_init_irq]
	/** Configure TC interrupts for TC TC_CHANNEL_CAPTURE only */
	NVIC_DisableIRQ(TC_IRQn);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 0);
	NVIC_EnableIRQ(TC_IRQn);
	//! [tc_capture_init_irq]

	/* Insert application code here, after the board has been initialized. */
	printf("\r\n[INFO] Board initialization complete\r\n");

	//! [tc_capture_init_module_irq]
	tc_enable_interrupt(TC, TC_CHANNEL_CAPTURE, TC_IER_LDRBS);

	/* Start the timer counter on TC TC_CHANNEL_CAPTURE */
	//! [tc_capture_start_now]
	tc_start(TC, TC_CHANNEL_CAPTURE);
	
	/* This skeleton code simply sets the LED to the state of the button. */
	while (1) {
	
		//! [tc_capture_init_module_irq]

		/* Is button pressed? */
		printf("\r\nRun main loop %d\r\n", gs_ul_captured_pulses);

		if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {
			/* Yes, so turn LED on. */
			ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
			
		} else {
			/* No, so turn LED off. */
			ioport_set_pin_level(LED_0_PIN, !LED_0_ACTIVE);
		}
	}
}
