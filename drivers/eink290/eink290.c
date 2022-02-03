#include "nrf.h"
#include "bsp.h"
#include "nrf_log.h"
#include "settings.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "eink290.h"

#include <openthread/platform/alarm-milli.h>

#define EINK290_SPI_INSTANCE 1
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(EINK290_SPI_INSTANCE);

// 4736 bytes
static uint8_t eink290_framebuffer[(EINK290_WIDTH / 8) * EINK290_HEIGHT] = {0};

static uint8_t logo[4736] = {0};

static volatile bool spi_xfer_done = true;

static void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
	spi_xfer_done = true;
}

static bool eink290_busy_wait(uint32_t timeout_ms)
{
	// 500uS steps
	timeout_ms *= 2;

	uint32_t time = 0;
	while (nrf_gpio_pin_read(EINK290_BUSY_PIN)) {
		if (time > timeout_ms)
			return false;
		time += 1;
		nrf_delay_us(500);
	}

	return true;
}

static bool eink290_spi_wait(uint32_t timeout_ms)
{
	// 500uS steps
	timeout_ms *= 2;

	uint32_t time = 0;
	while (!spi_xfer_done) {
		if (time > timeout_ms)
			return false;
		time += 1;
		nrf_delay_us(500);
	}

	return true;
}

static bool eink290_send_data_buffer(uint8_t *p_buffer, size_t size)
{
	nrf_gpio_pin_set(EINK290_DC_PIN);

	spi_xfer_done = false;

	nrfx_spim_xfer_desc_t const spim_xfer_desc = {
		.p_tx_buffer = p_buffer,
		.tx_length   = size,
		.p_rx_buffer = NULL,
		.rx_length   = 0,
	};

	ret_code_t result = nrfx_spim_xfer(&spi.u.spim, &spim_xfer_desc, 0);
	APP_ERROR_CHECK(result);

	if (!eink290_spi_wait(5000))
		return false;

	return true;
}

static bool eink290_send_command_data(uint8_t cmd, uint8_t *p_buffer, size_t size)
{
	nrf_gpio_pin_clear(EINK290_DC_PIN);

	spi_xfer_done = false;

	ret_code_t result = nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
	APP_ERROR_CHECK(result);

	if (!eink290_spi_wait(5000))
		return false;

	if (!p_buffer) {
		return true;
	}

	return eink290_send_data_buffer(p_buffer, size);
}

static bool eink290_send_command_and_byte(uint8_t cmd, uint8_t byte1)
{
	return eink290_send_command_data(cmd, &byte1, sizeof(byte1));
}

static bool eink290_hw_reset()
{
	nrf_gpio_pin_clear(EINK290_RST_PIN);
	nrf_delay_ms(10);
	nrf_gpio_pin_set(EINK290_RST_PIN);
	nrf_delay_ms(10);

	return true;
}

bool eink290_deep_sleep(uint8_t mode)
{
	return eink290_send_command_data(EINK290_ENTER_DEEP_SLEEP, &mode, sizeof(mode));
}

static bool eink290_set_window(uint8_t x_start, uint16_t y_start, uint8_t x_end, uint16_t y_end)
{
	eink290_window_x_t wx = {x_start >> 3, x_end >> 3};
	eink290_window_y_t wy = {y_start, y_end};

	if (!eink290_send_command_data(EINK290_SET_RAM_X_WINDOW, (uint8_t *)&wx, sizeof(wx)))
		return false;
	if (!eink290_send_command_data(EINK290_SET_RAM_Y_WINDOW, (uint8_t *)&wy, sizeof(wy)))
		return false;

	return true;
}

static bool eink290_set_cursor(uint8_t x, uint16_t y)
{
	eink290_cursor_x_t cx = {x >> 3};
	eink290_cursor_y_t cy = {y};

	if (!eink290_send_command_data(EINK290_SET_RAM_X_ADDRESS_CURSOR, (uint8_t *)&cx, sizeof(cx)))
		return false;
	if (!eink290_send_command_data(EINK290_SET_RAM_Y_ADDRESS_CURSOR, (uint8_t *)&cy, sizeof(cy)))
		return false;

	return true;
}

bool eink290_write_lut(eink290_luts_t *p_lut)
{
	return eink290_send_command_data(EINK290_WRITE_LUT, (uint8_t *)p_lut, sizeof(eink290_luts_t));
}

static bool eink290_clear(uint8_t color, uint8_t width, uint16_t height)
{
	eink290_clear_t clr = {0};
	clr.color = color ? 1 : 0;
	clr.height = 0b110; // 296 according to ssd1680 datasheet
	clr.width = 0b101; // 176 according to ssd1680 datasheet

	if (height < 296) {
		for (uint8_t i = 0b101; i >= 0; i--) {
			if (!i || (height >= (8 << i))) {
				clr.height = i;
				break;
			}
		}
	}

	if (width < 176) {
		for (uint8_t i = 0b100; i >= 0; i--) {
			if (!i || (width >= (8 << i))) {
				clr.width = i;
				break;
			}
		}
	}

	if (!eink290_send_command_data(EINK290_CLEAR_AREA, (uint8_t *)&clr, sizeof(clr)))
		return false;
	if (!eink290_busy_wait(5000))
		return false;

	return true;
}

bool eink290_fill_test_pattern1() {
	if (!eink290_set_window(0, 0, EINK290_WIDTH - 1, EINK290_HEIGHT - 1))
		return false;
	if (!eink290_set_cursor(0, 0))
		return false;

	for (uint32_t i = 0; i < sizeof(eink290_framebuffer); i++) {
		uint32_t y = (i * 8) / 128; // 0 - 295
		uint32_t x = (i * 8) % 128; // 0 - 127
		uint8_t color = 0x00;
		// FF 55
		// F0 0F
		// 00 CC
		if (x < 64) {
			if (y < 100)
				color = 0xFF;
			else if (y > 200)
				color = 0x00;
			else
				color = 0xF0;
		} else {
			if (y < 100)
				color = 0x55;
			else if (y > 200)
				color = 0xCC;
			else
				color = 0x0F;
		}
		eink290_framebuffer[i] = color;
	}

	return eink290_send_command_data(EINK290_WRITE_RAM, eink290_framebuffer, sizeof(eink290_framebuffer));
}

bool eink290_fill_test_pattern2() {
	if (!eink290_set_window(0, 0, EINK290_WIDTH - 1, EINK290_HEIGHT - 1))
		return false;
	if (!eink290_set_cursor(0, 0))
		return false;

	for (uint32_t i = 0; i < sizeof(eink290_framebuffer); i++) {
		uint32_t y = (i * 8) / 128; // 0 - 295
		uint32_t x = (i * 8) % 128; // 0 - 127
		uint8_t color = 0x00;
		// FF 55
		// F0 0F
		// 00 CC
		if (x < 16 || x > 111) {
			if (y < 16)
				color = 0xFF;
			else if (y > 280)
				color = 0x00;
			else
				color = 0xF0;
		} else {
			if (y < 16)
				color = 0x55;
			else if (y > 280)
				color = 0xCC;
			else
				color = 0x0F;
		}
		eink290_framebuffer[i] = color;
	}

	return eink290_send_command_data(EINK290_WRITE_RAM, eink290_framebuffer, sizeof(eink290_framebuffer));
}

bool eink290_fill_test_pattern3() {
	if (!eink290_set_window(0, 0, EINK290_WIDTH - 1, EINK290_HEIGHT - 1))
		return false;
	if (!eink290_set_cursor(0, 0))
		return false;

	memcpy(eink290_framebuffer, logo, 4736);

	return eink290_send_command_data(EINK290_WRITE_RAM, eink290_framebuffer, sizeof(eink290_framebuffer));
}

bool eink290_duc2_and_activate(uint8_t duc2_options)
{
	if (!eink290_send_command_and_byte(EINK290_DISPLAY_UPDATE_CONTROL_2, duc2_options))
		return false;

	if (!eink290_send_command_data(EINK290_MASTER_ACTIVATION, NULL, 0))
		return false;

	if (!eink290_busy_wait(5000))
		return false;

	return true;
}

bool eink290_initialize()
{
	if (!eink290_hw_reset())
		return false;
	if (!eink290_busy_wait(5000))
		return false;

	if (!eink290_send_command_data(EINK290_SW_RESET, NULL, 0))
		return false;
	if (!eink290_busy_wait(5000))
		return false;

	eink290_driver_output_t driver_output = {0};
	driver_output.mux_lines = EINK290_HEIGHT - 1; // A[8:0]= 127h [POR], 296 MUX MUX Gate lines setting as (A[8:0] + 1).
	if (!eink290_send_command_data(EINK290_DRIVER_OUTPUT_CONTROL, (uint8_t *)&driver_output, sizeof(driver_output)))
		return false;

	eink290_data_entry_mode_t data_entry = {0};
	data_entry.x_increment = EINK290_DATA_ENTRY_MODE_INCREMENT;
	data_entry.y_increment = EINK290_DATA_ENTRY_MODE_INCREMENT;
	data_entry.direction = 1;
	if (!eink290_send_command_data(EINK290_DATA_ENTRY_MODE_SETTING, (uint8_t *)&data_entry, sizeof(data_entry)))
		return false;

//	eink290_display_option_t display_option = {0};
//	display_option.pingpong_mode2_enable = 1;
//	if (!eink290_send_command_data(EINK290_DISPLAY_OPTION, (uint8_t *)&display_option, sizeof(display_option)))
//		return false;

	if (!eink290_set_window(0, 0, EINK290_WIDTH - 1, EINK290_HEIGHT - 1))
		return false;

	eink290_border_waveform_control_t border = {0};
	border.vbd_transition = EINK290_VBD_TRANSITION_LUT1; // LUT1
	border.gs_transition = EINK290_GS_TRANSITION_LUT; // Follow LUT
	if (!eink290_send_command_data(EINK290_BORDER_WAVEFORM_CONTROL, (uint8_t *)&border, sizeof(border)))
		return false;

	eink290_display_update_control_1_t up1 = {0};
	up1.source_output_mode = EINK290_DUC1_SOURCE_OUTPUT_8_167; // 1 = Available Source from S8 to S167
	if (!eink290_send_command_data(EINK290_DISPLAY_UPDATE_CONTROL_1, (uint8_t *)&up1, sizeof(up1)))
		return false;

	if (!eink290_send_command_and_byte(EINK290_TEMPERATURE_SENSOR_CONTROL, EINK290_USE_INTERNAL_TEMP_SENSOR))
		return false;

	return true;
}

bool eink290_test()
{
	if (!eink290_fill_test_pattern3())
		return false;
	if (!eink290_duc2_and_activate(EINK290_DUC2_DISPLAY_MODE_1))
		return false;

	if (!eink290_fill_test_pattern1())
		return false;
	if (!eink290_duc2_and_activate(EINK290_DUC2_DISPLAY_MODE_1))
		return false;

	if (!eink290_clear(1, EINK290_WIDTH, EINK290_HEIGHT))
		return false;
	if (!eink290_duc2_and_activate(EINK290_DUC2_DISPLAY_MODE_1))
		return false;

/*		
	eink290_luts_t l = {0};
	// black
	l.vs_lut0.g0.vsB = EINK290_LUT_VS_VSL;
	l.vs_lut0.g0.vsD = EINK290_LUT_VS_VSL;
	l.vs_lut0.g1.vsB = EINK290_LUT_VS_VSH1;
	l.vs_lut0.g1.vsD = EINK290_LUT_VS_VSH1;
	// white
	l.vs_lut1.g0.vsB = EINK290_LUT_VS_VSH1;
	l.vs_lut1.g0.vsD = EINK290_LUT_VS_VSH1;
	l.vs_lut1.g1.vsA = EINK290_LUT_VS_VSL;
	l.vs_lut1.g1.vsC = EINK290_LUT_VS_VSL;
	// group0
	l.g0.tp_a = 4; // 4
	l.g0.tp_b = 24; // 24
	l.g0.tp_c = 4; // 4
	l.g0.tp_d = 4; // 24
	l.g0.rp = 5; // 1
	l.fr0 = 2; // 4
	// group1
	l.g1.tp_a = 10; // 10
	l.g1.tp_b = 10; // 10
	l.g1.tp_c = 10; // 10
	l.g1.tp_d = 10; // 10
	l.g1.rp = 1;
	l.fr1 = 2;

	if (!eink290_write_lut(&l))
		return false;
*/

	return true;
}

ret_code_t eink290_hw_init()
{
	nrf_gpio_cfg_input(EINK290_BUSY_PIN, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_pin_set(EINK290_DC_PIN);
	nrf_gpio_cfg_output(EINK290_DC_PIN);

	nrf_gpio_pin_set(EINK290_RST_PIN);
	nrf_gpio_cfg_output(EINK290_RST_PIN);

	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin = EINK290_SPI_CS_PIN;
	spi_config.miso_pin = EINK290_SPI_MISO_PIN;
	spi_config.mosi_pin = EINK290_SPI_MOSI_PIN;
	spi_config.sck_pin = EINK290_SPI_SCK_PIN;
	spi_config.frequency = NRF_DRV_SPI_FREQ_8M;

	APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

	if (!eink290_initialize())
		return NRF_ERROR_INTERNAL;

	memset(eink290_framebuffer, 0xFF, sizeof(eink290_framebuffer));

	return NRF_SUCCESS;
}
