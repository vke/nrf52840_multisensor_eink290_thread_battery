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

// 0 = vss_dcvcom, 1 = vsh1_vsh1_dcvcom, 2 = vsl_vsl_dcvcom, 3 = vsh2_na
#define EINK290_LUT_VS_VSS                          0b00
#define EINK290_LUT_VS_VSH1                         0b01
#define EINK290_LUT_VS_VSL                          0b10
#define EINK290_LUT_VS_VSH2                         0b11

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
#define EINK290_WRITE_LUT                           0x32
#define EINK290_DISPLAY_OPTION                      0x37
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

typedef struct {
	uint8_t unused1:7;
	// A[7] Spare VCOM OTP selection
	uint8_t use_spare_otp:1;
	// B[7:0] Display Mode for WS[7:0]
	uint8_t display_mode_ws_0:1;
	uint8_t display_mode_ws_1:1;
	uint8_t display_mode_ws_2:1;
	uint8_t display_mode_ws_3:1;
	uint8_t display_mode_ws_4:1;
	uint8_t display_mode_ws_5:1;
	uint8_t display_mode_ws_6:1;
	uint8_t display_mode_ws_7:1;
	// C[7:0] Display Mode for WS[15:8]
	uint8_t display_mode_ws_8:1;
	uint8_t display_mode_ws_9:1;
	uint8_t display_mode_ws_10:1;
	uint8_t display_mode_ws_11:1;
	uint8_t display_mode_ws_12:1;
	uint8_t display_mode_ws_13:1;
	uint8_t display_mode_ws_14:1;
	uint8_t display_mode_ws_15:1;
	// D[7:0] Display Mode for WS[23:16]
	uint8_t display_mode_ws_16:1;
	uint8_t display_mode_ws_17:1;
	uint8_t display_mode_ws_18:1;
	uint8_t display_mode_ws_19:1;
	uint8_t display_mode_ws_20:1;
	uint8_t display_mode_ws_21:1;
	uint8_t display_mode_ws_22:1;
	uint8_t display_mode_ws_23:1;
	// E[7:0] Display Mode for WS[31:24]
	uint8_t display_mode_ws_24:1;
	uint8_t display_mode_ws_25:1;
	uint8_t display_mode_ws_26:1;
	uint8_t display_mode_ws_27:1;
	uint8_t display_mode_ws_28:1;
	uint8_t display_mode_ws_29:1;
	uint8_t display_mode_ws_30:1;
	uint8_t display_mode_ws_31:1;
	// F[3:0] Display Mode for WS[35:32]
	uint8_t display_mode_ws_32:1;
	uint8_t display_mode_ws_33:1;
	uint8_t display_mode_ws_34:1;
	uint8_t display_mode_ws_35:1;
	uint8_t unused2:2;
	// F[6]: PingPong for Display Mode 2
	uint8_t pingpong_mode2_enable:1;
	uint8_t unused3:1;
	// G[7:0]~J[7:0] module ID/waveform version.
	uint8_t version0;
	uint8_t version1;
	uint8_t version2;
	uint8_t version3;
} eink290_display_option_t;

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

typedef struct {
	uint8_t vsD:2; // 0 = vss_dcvcom, 1 = vsh1_vsh1_dcvcom, 2 = vsl_vsl_dcvcom, 3 = vsh2_na
	uint8_t vsC:2; // 0 = vss_dcvcom, 1 = vsh1_vsh1_dcvcom, 2 = vsl_vsl_dcvcom, 3 = vsh2_na
	uint8_t vsB:2; // 0 = vss_dcvcom, 1 = vsh1_vsh1_dcvcom, 2 = vsl_vsl_dcvcom, 3 = vsh2_na
	uint8_t vsA:2; // 0 = vss_dcvcom, 1 = vsh1_vsh1_dcvcom, 2 = vsl_vsl_dcvcom, 3 = vsh2_na
} eink290_ws_vs_t;

typedef struct {
	eink290_ws_vs_t g0;
	eink290_ws_vs_t g1;
	eink290_ws_vs_t g2;
	eink290_ws_vs_t g3;
	eink290_ws_vs_t g4;
	eink290_ws_vs_t g5;
	eink290_ws_vs_t g6;
	eink290_ws_vs_t g7;
	eink290_ws_vs_t g8;
	eink290_ws_vs_t g9;
	eink290_ws_vs_t g10;
	eink290_ws_vs_t g11;
} eink290_ws_vs_set_t;

typedef struct {
	uint8_t tp_a; // represents the phase length set by the number of frame, 0 = skip
	uint8_t tp_b; // represents the phase length set by the number of frame, 0 = skip
	uint8_t sr_ab; // represent the state repeat counting number. SR[nXY] = 0 indicates that the repeat times =1, SR[nXY] = 1 indicates that the repeat times = 2, and so on. The maximum repeat times is 256.
	uint8_t tp_c; // represents the phase length set by the number of frame, 0 = skip
	uint8_t tp_d; // represents the phase length set by the number of frame, 0 = skip
	uint8_t sr_cd; // represent the state repeat counting number. SR[nXY] = 0 indicates that the repeat times =1, SR[nXY] = 1 indicates that the repeat times = 2, and so on. The maximum repeat times is 256.
	uint8_t rp; // represents the repeat counting number for the Group. RP[n] = 0 indicates that the repeat times =1, RP[n] = 1 indicates that the repeat times = 2, and so on. The maximum repeat times is 256.
} eink290_ws_group_params_t;

typedef struct {
	eink290_ws_vs_set_t vs_lut0; // 0 - 11
	eink290_ws_vs_set_t vs_lut1; // 12 - 23
	eink290_ws_vs_set_t vs_lut2; // 24 - 35
	eink290_ws_vs_set_t vs_lut3; // 36 - 47
	eink290_ws_vs_set_t vs_lut4;  // 48 - 59
	eink290_ws_group_params_t g0; // 60 - 66
	eink290_ws_group_params_t g1; // 67 - 73
	eink290_ws_group_params_t g2; // 74 - 80
	eink290_ws_group_params_t g3; // 81 - 87
	eink290_ws_group_params_t g4; // 88 - 94
	eink290_ws_group_params_t g5; // 95 - 101
	eink290_ws_group_params_t g6; // 102 - 108
	eink290_ws_group_params_t g7; // 109 - 115
	eink290_ws_group_params_t g8; // 116 - 122
	eink290_ws_group_params_t g9; // 123 - 129
	eink290_ws_group_params_t g10; // 130 - 136
	eink290_ws_group_params_t g11; // 137 - 143
	uint8_t fr1:4;
	uint8_t fr0:4; // 144
	uint8_t fr3:4;
	uint8_t fr2:4; // 145
	uint8_t fr5:4;
	uint8_t fr4:4; // 146
	uint8_t fr7:4;
	uint8_t fr6:4; // 147
	uint8_t fr9:4;
	uint8_t fr8:4; // 148
	uint8_t fr11:4;
	uint8_t fr10:4; // 149
	uint8_t xon_3cd:1;
	uint8_t xon_3ab:1;
	uint8_t xon_2cd:1;
	uint8_t xon_2ab:1;
	uint8_t xon_1cd:1;
	uint8_t xon_1ab:1;
	uint8_t xon_0cd:1;
	uint8_t xon_0ab:1; // 150
	uint8_t xon_7cd:1;
	uint8_t xon_7ab:1;
	uint8_t xon_6cd:1;
	uint8_t xon_6ab:1;
	uint8_t xon_5cd:1;
	uint8_t xon_5ab:1;
	uint8_t xon_4cd:1;
	uint8_t xon_4ab:1; // 151
	uint8_t xon_11cd:1;
	uint8_t xon_11ab:1;
	uint8_t xon_10cd:1;
	uint8_t xon_10ab:1;
	uint8_t xon_9cd:1;
	uint8_t xon_9ab:1;
	uint8_t xon_8cd:1;
	uint8_t xon_8ab:1; // 152
} eink290_luts_t;

typedef struct {
	eink290_luts_t luts; // 0 - 152
	uint8_t eopt; // 153
	uint8_t vgh; // 154
	uint8_t vsh1; // 155
	uint8_t vsh2; // 156
	uint8_t vsl; // 157
	uint8_t vcom; // 158
} eink290_ws_t;

#pragma pack(pop)

bool eink290_deep_sleep(uint8_t mode);
bool eink290_test();
bool eink290_initialize();
ret_code_t eink290_hw_init();

#endif // _EINK290_H__
