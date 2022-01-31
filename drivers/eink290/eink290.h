#ifndef _EINK290_H__
#define _EINK290_H__

#define EINK290_WIDTH       128
#define EINK290_HEIGHT      296

#define EINK290_DEEP_SLEEP_MODE_1                   0b01
#define EINK290_DEEP_SLEEP_MODE_2                   0b11

#define EINK290_DATA_ENTRY_MODE_INCREMENT           1
#define EINK290_DATA_ENTRY_MODE_DECREMENT           0

#define EINK290_VBD_TRANSITION_LUT0                 0
#define EINK290_VBD_TRANSITION_LUT1                 1
#define EINK290_VBD_TRANSITION_LUT2                 2
#define EINK290_VBD_TRANSITION_LUT3                 3

#define EINK290_GS_TRANSITION_LUT_RED               0
#define EINK290_GS_TRANSITION_LUT                   1

#define EINK290_DUC1_SOURCE_OUTPUT_0_175            0
#define EINK290_DUC1_SOURCE_OUTPUT_8_167            1

#define EINK290_DUC2_DISABLE_CLOCK                  0x01
#define EINK290_DUC2_DISABLE_ANALOG                 0x02
#define EINK290_DUC2_DISPLAY_PATTERN                0x04
#define EINK290_DUC2_DISPLAY_INITIAL                0x08
#define EINK290_DUC2_LOAD_LUT                       0x10
#define EINK290_DUC2_LOAD_TEMPERATURE               0x20
#define EINK290_DUC2_ENABLE_ANALOG                  0x40
#define EINK290_DUC2_ENABLE_CLOCK                   0x80

#define EINK290_DUC2_DISPLAY_MODE_1                 (EINK290_DUC2_DISABLE_CLOCK | EINK290_DUC2_DISABLE_ANALOG | EINK290_DUC2_DISPLAY_PATTERN | EINK290_DUC2_LOAD_LUT | EINK290_DUC2_LOAD_TEMPERATURE | EINK290_DUC2_ENABLE_ANALOG | EINK290_DUC2_ENABLE_CLOCK)
#define EINK290_DUC2_DISPLAY_MODE_2                 (EINK290_DUC2_DISABLE_CLOCK | EINK290_DUC2_DISABLE_ANALOG | EINK290_DUC2_DISPLAY_PATTERN | EINK290_DUC2_DISPLAY_INITIAL | EINK290_DUC2_LOAD_LUT | EINK290_DUC2_LOAD_TEMPERATURE | EINK290_DUC2_ENABLE_ANALOG | EINK290_DUC2_ENABLE_CLOCK)

#define EINK290_USE_INTERNAL_TEMP_SENSOR            0x80

#define EINK290_DRIVER_OUTPUT_CONTROL               0x01
#define EINK290_ENTER_DEEP_SLEEP                    0x10
#define EINK290_DATA_ENTRY_MODE_SETTING             0x11
#define EINK290_SW_RESET                            0x12
#define EINK290_TEMPERATURE_SENSOR_CONTROL          0x18
#define EINK290_MASTER_ACTIVATION                   0x20
#define EINK290_DISPLAY_UPDATE_CONTROL_1            0x21
#define EINK290_DISPLAY_UPDATE_CONTROL_2            0x22
#define EINK290_WRITE_RAM                           0x24
#define EINK290_BORDER_WAVEFORM_CONTROL             0x3C
#define EINK290_SET_RAM_X_WINDOW                    0x44
#define EINK290_SET_RAM_Y_WINDOW                    0x45
#define EINK290_CLEAR_AREA                          0x47
#define EINK290_SET_RAM_X_ADDRESS_CURSOR            0x4E
#define EINK290_SET_RAM_Y_ADDRESS_CURSOR            0x4F

#pragma pack(push, 1)

// Gate setting A[8:0]= 127h [POR], 296 MUX MUX Gate lines setting as (A[8:0] + 1).
typedef struct {
	uint16_t mux_lines:9;
	uint16_t unused1:7;
	uint8_t tb:1;
	uint8_t sm:1;
	uint8_t gd:1;
	uint8_t unused2:5;
} eink290_driver_output_t;

// Define data entry sequence A[2:0] = 011 [POR]
typedef struct {
	// 1 = x increment, 0 = x decrement
	uint8_t x_increment:1;
	// 1 = y increment, 0 = y decrement
	uint8_t y_increment:1;
	// Set the direction in which the address counter is updated automatically after data are written to the RAM.
	// 0 = the address counter is updated in the X direction. [POR]
	// 1 = the address counter is updated in the Y direction.
	uint8_t direction:1;
	uint8_t unused:5;
} eink290_data_entry_mode_t;

typedef struct {
	// A [1:0] GS Transition setting for VBD
	// 0 = LUT0 ... 3 = LUT3
	uint8_t vbd_transition:2;
	// A[2] GS Transition control
	// 0 = Follow LUT (Output VCOM @ RED)
	// 1 = Follow LUT
	uint8_t gs_transition:1;
	// A [5:4] Fix Level Setting for VBD
	// 0b00 = VSS
	// 0b01 = VSH1
	// 0b10 = VSL
	// 0b11 = VSH2
	uint8_t vbd_level:2;
	// A [7:6] :Select VBD option
	// 0b00 = GS Transition, Defined in A[2] and A[1:0]
	// 0b01 = Fix Level, Defined in A[5:4]
	// 0b10 = VCOM
	// 0b11 = HiZ [POR]
	uint8_t vbd_option:2;
} eink290_border_waveform_control_t;

typedef struct {
	// A[3:0] BW RAM option
	// 0b0000 = Normal
	// 0b0100 = Bypass RAM content as 0
	// 0b1000 = Inverse RAM content
	uint8_t bw_ram_option:4;
	// A[7:4] Red RAM option
	// 0b0000 = Normal
	// b00100 = Bypass RAM content as 0
	// b01000 = Inverse RAM content
	uint8_t red_ram_option:4;
	uint8_t unused:7;
	// B[7] Source Output Mode
	// 0 = Available Source from S0 to S175
	// 1 = Available Source from S8 to S167
	uint8_t source_output_mode:1;
} eink290_display_update_control_1_t;

// Specify the start/end positions of the window address in the X direction by an address unit for RAM
// A[5:0]: XSA[5:0], XStart, POR = 00h
// B[5:0]: XEA[5:0], XEnd, POR = 15h
typedef struct {
	uint8_t x_start;
	uint8_t x_end;
} eink290_window_x_t;

// Specify the start/end positions of the window address in the Y direction by an address unit for RAM
// A[8:0]: YSA[8:0], YStart, POR = 000h
// B[8:0]: YEA[8:0], YEnd, POR = 127h
typedef struct {
	uint16_t y_start;
	uint16_t y_end;
} eink290_window_y_t;

// Make initial settings for the RAM X address in the address counter (AC) A[5:0]: 00h [POR].
typedef struct {
	uint8_t x_pos;
} eink290_cursor_x_t;

// Make initial settings for the RAM Y address in the address counter (AC) A[8:0]: 000h [POR].
typedef struct {
	uint16_t y_pos;
} eink290_cursor_y_t;

// Auto Write B/W RAM for Regular Pattern A[7:0] = 00h [POR]
typedef struct {
	uint8_t width:3;
	uint8_t unused:1;
	uint8_t height:3;
	uint8_t color:1;
} eink290_clear_t;

#pragma pack(pop)

bool eink290_deep_sleep(uint8_t mode);
bool eink290_test();
ret_code_t eink290_init();

#endif // _EINK290_H__
