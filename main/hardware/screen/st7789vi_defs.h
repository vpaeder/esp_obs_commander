/** \file st7789vi_defs.h
 *  \brief Header file containing definitions for ST7789VI screen controller.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once

// ST7789VI commands (from datasheet v1.5)
#define ST7789VI_NOP                0x00 ///< No op
#define ST7789VI_SWRESET            0x01 ///< Software Reset
#define ST7789VI_RDDID              0x04 ///< Read Display ID
#define ST7789VI_RDDST              0x09 ///< Read Display Status
#define ST7789VI_RDDPM              0x0A ///< Read Display Power Mode
#define ST7789VI_RDDMADCTL          0x0B ///< Read Display MADCTL
#define ST7789VI_RDDCOLMOD          0x0C ///< Read Display Pixel Format
#define ST7789VI_RDDIM              0x0D ///< Read Display Image Mode
#define ST7789VI_RDDSM              0x0E ///< Read Display Signal Mode
#define ST7789VI_RDDSDR             0x0F ///< Read Display Self-Diagnostic Result

#define ST7789VI_SLPIN              0x10 ///< Sleep In
#define ST7789VI_SLPOUT             0x11 ///< Sleep Out
#define ST7789VI_PTLON              0x12 ///< Partial Display Mode On
#define ST7789VI_NORON              0x13 ///< Partial Display Mode Off
#define ST7789VI_INVOFF             0x20 ///< Display Inversion Off
#define ST7789VI_INVON              0x21 ///< Display Inversion On
#define ST7789VI_GAMSET             0x26 ///< Gamma Set
#define ST7789VI_DISPOFF            0x28 ///< Display Off
#define ST7789VI_DISPON             0x29 ///< Display On
#define ST7789VI_CASET              0x2A ///< Column Address Set
#define ST7789VI_RASET              0x2B ///< Row Address Set
#define ST7789VI_RAMWR              0x2C ///< Memory Write
#define ST7789VI_RAMRD              0x2E ///< Memory Read
#define ST7789VI_PTLAR              0x30 ///< Partial Area
#define ST7789VI_VSCRDEF            0x33 ///< Vertical Scrolling Definition
#define ST7789VI_TEOFF              0x34 ///< Tearing Effect Line Off
#define ST7789VI_TEON               0x35 ///< Tearing Effect Line On
#define ST7789VI_MADCTL             0x36 ///< Memory Data Access Control
#define ST7789VI_VSCSAD             0x37 ///< Vertical Scroll Start Address of RAM
#define ST7789VI_IDMOFF             0x38 ///< Idle Mode Off
#define ST7789VI_IDMON              0x39 ///< Idle Mode On
#define ST7789VI_COLMOD             0x3A ///< Interface Pixel Format
#define ST7789VI_WRMEMC             0x3C ///< Write Memory Continue
#define ST7789VI_RDMEMC             0x3E ///< Read Memory Continue
#define ST7789VI_STE                0x44 ///< Set Tear Scanline
#define ST7789VI_GSCAN              0x45 ///< Get Scanline
#define ST7789VI_WRDISBV            0x51 ///< Write Display Brightness
#define ST7789VI_RDDISBV            0x52 ///< Read Display Brightness Value
#define ST7789VI_WRCTRLD            0x53 ///< Write CTRL Display
#define ST7789VI_RDCTRLD            0x54 ///< Read CTRL Value Display
#define ST7789VI_WRCACE             0x55 ///< Write Content Adaptive Brightness Control and Color Enhancement
#define ST7789VI_RDCABC             0x56 ///< Read Content Adaptive Brightness Control
#define ST7789VI_WRCABCMB           0x5e ///< Write CABC Minimum Brightness
#define ST7789VI_RDCABCMB           0x5f ///< Read CABC Minimum Brightness
#define ST7789VI_RDABCSDR           0x68 ///< Read Automatic Brightness Self-Diagnostic Result

#define ST7789VI_RAMCTRL            0xB0 ///< RAM Control
#define ST7789VI_RGBCTRL            0xB1 ///< RGB Interface Control
#define ST7789VI_PORCTRL            0xB2 ///< Porch Setting
#define ST7789VI_FRCTRL1            0xB3 ///< Frame Rate Control 1 (In partial mode/ idle colors)
#define ST7789VI_PARCTRL            0xB5 ///< Partial mode Control
#define ST7789VI_GCTRL              0xB7 ///< Gate Control
#define ST7789VI_GTADJ              0xB8 ///< Gate On Timing Adjustment
#define ST7789VI_DGMEN              0xBA ///< Digital Gamma Enable
#define ST7789VI_VCOMS              0xBB ///< VCOMS Setting
#define ST7789VI_LCMCTRL            0xC0 ///< LCM Control
#define ST7789VI_IDSET              0xC1 ///< ID Code Setting
#define ST7789VI_VDVVRHEN           0xC2 ///< VDV and VRH Command Enable
#define ST7789VI_VRHS               0xC3 ///< VRH Set
#define ST7789VI_VDVS               0xC4 ///< VDV Set
#define ST7789VI_VCMOFFSET          0xC5 ///< VCOMS Offset Set
#define ST7789VI_FRCTRL2            0xC6 ///< Frame Rate Control in Normal Mode
#define ST7789VI_CABCCTRL           0xC7 ///< CABC Control
#define ST7789VI_REGSEL1            0xC8 ///< Register Value Selection 1
#define ST7789VI_REGSEL2            0xCA ///< Register Value Selection 2
#define ST7789VI_PWMFRSEL           0xCC ///< PWM Frequency Selection
#define ST7789VI_PWCTRL1            0xD0 ///< Power Control 1
#define ST7789VI_VAPVANEN           0xD2 ///< Enable VAP/VAN signal output
#define ST7789VI_RDID1              0xDA ///< Read ID1
#define ST7789VI_RDID2              0xDB ///< Read ID2
#define ST7789VI_RDID3              0xDC ///< Read ID3
#define ST7789VI_CMD2EN             0xDF ///< Command 2 Enable
#define ST7789VI_PVGAMCTRL          0xE0 ///< Positive Voltage Gamma Control
#define ST7789VI_NVGAMCTRL          0xE1 ///< Negative Voltage Gamma Control
#define ST7789VI_DGMLUTR            0xE2 ///< Digital Gamma Look-up Table for Red
#define ST7789VI_DGMLUTB            0xE3 ///< Digital Gamma Look-up Table for Blue
#define ST7789VI_GATECTRL           0xE4 ///< Gate Control
#define ST7789VI_SPI2EN             0xE7 ///< SPI2 Enable
#define ST7789VI_PWCTRL2            0xE8 ///< Power Control 2
#define ST7789VI_EQCTRL             0xE9 ///< Equalize time control
#define ST7789VI_PROMCTRL           0xEC ///< Program Mode Control
#define ST7789VI_PROMEN             0xFA ///< Program Mode Enable
#define ST7789VI_NVMSET             0xFC ///< NVM Setting
#define ST7789VI_PROMACT            0xFE ///< Program action 

// Parameters for ST7789VI_GAMSET command
#define ST7789VI_GAMSET_GC0         0x01 ///< Gamma Curve 1 (G2.2)
#define ST7789VI_GAMSET_GC1         0x02 ///< Gamma Curve 2 (G1.8)
#define ST7789VI_GAMSET_GC2         0x04 ///< Gamma Curve 3 (G2.5)
#define ST7789VI_GAMSET_GC3         0x08 ///< Gamma Curve 4 (G1.0)

// Parameters for ST7789VI_MADCTL command
#define ST7789VI_MADCTL_MY          0x80 ///< Page Address Order ('0': Top to Bottom, '1': Bottom to Top)
#define ST7789VI_MADCTL_MX          0x40 ///< Column Address Order ('0': Left to Right, '1': Right to Left)
#define ST7789VI_MADCTL_MV          0x20 ///< Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode)
#define ST7789VI_MADCTL_ML          0x10 ///< Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = LCD Refresh Bottom to Top)
#define ST7789VI_MADCTL_RGB         0x08 ///< RGB/BGR Order ('0' = RGB, '1' = BGR)
#define ST7789VI_MADCTL_MH          0x04 ///< Display Data Latch Order ('0' = LCD Refresh Left to Right, '1' = LCD Refresh Right to Left)

// Parameters for ST7789VI_COLMOD command
#define ST7789VI_COLMOD_INT_65K     0x50 ///< RGB interface color format: 65K
#define ST7789VI_COLMOD_INT_262K    0x60 ///< RGB interface color format: 262K
#define ST7789VI_COLMOD_CTRL_12BIT  0x03 ///< Control interface color format: 12bit/pixel
#define ST7789VI_COLMOD_CTRL_16BIT  0x05 ///< Control interface color format: 16bit/pixel
#define ST7789VI_COLMOD_CTRL_18BIT  0x06 ///< Control interface color format: 18bit/pixel
#define ST7789VI_COLMOD_CTRL_24BIT  0x07 ///< Control interface color format: 24bit/pixel, truncated

// Parameters for ST7789VI_WRCACE command
#define ST7789VI_WRCACE_C00         0x00 ///< Brightness control off
#define ST7789VI_WRCACE_C01         0x01 ///< Brightness control: User Interface Mode
#define ST7789VI_WRCACE_C10         0x02 ///< Brightness control: Still Picture
#define ST7789VI_WRCACE_C11         0x03 ///< Brightness control: Moving Image
#define ST7789VI_WRCACE_CECTRL      0x80 ///< Color ehancement control ('0' = Color Enhancement Off, '1' = Color Enhancement On)
#define ST7789VI_WRCACE_CE00        0x00 ///< Color ehancement: Low enhancement
#define ST7789VI_WRCACE_CE01        0x10 ///< Color ehancement: Medium enhancement
#define ST7789VI_WRCACE_CE11        0x30 ///< Color ehancement: High enhancement

// Parameters for ST7789VI_RAMCTRL command
#define ST7789VI_RAMCTRL_RM         0x10 ///< RAM access selection ('0' = from MCU interface, '1' = from RGB interface)
#define ST7789VI_RAMCTRL_DM_MCU     0x00 ///< Display operation: MCU interface
#define ST7789VI_RAMCTRL_DM_RGB     0x01 ///< Display operation: RGB interface
#define ST7789VI_RAMCTRL_DM_VSYNC   0x02 ///< Display operation: VSYNC interface
#define ST7789VI_RAMCTRL_ENDIAN     0x08 ///< Endian ('0' = Big endian, '1' = Little endian)
#define ST7789VI_RAMCTRL_RIM        0x04 ///< RGB interface bus width ('0' = 18bit width, '1' = 6bit width)
// Data translation from 65k format to 4k format (see datasheet pp.259-260 for detailed description)
#define ST7789VI_RAMCTRL_EPF00      0x00 ///< see datasheet pp.259-260 for detailed description
#define ST7789VI_RAMCTRL_EPF01      0x10 ///< see datasheet pp.259-260 for detailed description
#define ST7789VI_RAMCTRL_EPF10      0x20 ///< see datasheet pp.259-260 for detailed description
#define ST7789VI_RAMCTRL_EPF11      0x30 ///< see datasheet pp.259-260 for detailed description
// TODO: add MDT command (section 8.8 Data Color Coding)

// Parameters for ST7789VI_RGBCTRL command
#define ST7789VI_RGBCTRL_WO         0x80 ///< Direct RGB mode ('0' = Memory, '1' = Shift register)
#define ST7789VI_RGBCTRL_RCM_MCU    0x20 ///< RGB interface: MCU
#define ST7789VI_RGBCTRL_RCM_DE     0x40 ///< RGB interface: DE
#define ST7789VI_RGBCTRL_RCM_HV     0x60 ///< RGB interface: HV
#define ST7789VI_RGBCTRL_VSPL       0x08 ///< Signal polarity of VSYNC pin ('0' = active low, '1' = active high)
#define ST7789VI_RGBCTRL_HSPL       0x04 ///< Signal polarity of HSYNC pin ('0' = active low, '1' = active high)
#define ST7789VI_RGBCTRL_DPL        0x02 ///< Signal polarity of DOTCLK pin ('0' = positive edge triggering, '1' = negative edge triggering)
#define ST7789VI_RGBCTRL_EPL        0x01 ///< Signal polarity of DOTCLK pin ('0' = data written when ENABLE = '1', '1' = data written when ENABLE = '0')

// Parameters for ST7789VI_PORCTRL command
#define ST7789VI_PORCTRL_PSEN       0x01 ///< Enable separate porch control ('0' = disable, '1' = enable)

// Parameters for ST7789VI_FRCTRL1 command
#define ST7789VI_FRCTRL1_FRSEN      0x10 ///< Enable separate frame rate control ('0' = disable, '1' = enable)
#define ST7789VI_FRCTRL1_DIV1       0x00 ///< Frame rate divider set to 1
#define ST7789VI_FRCTRL1_DIV2       0x01 ///< Frame rate divider set to 2
#define ST7789VI_FRCTRL1_DIV4       0x02 ///< Frame rate divider set to 4
#define ST7789VI_FRCTRL1_DIV8       0x03 ///< Frame rate divider set to 8
#define ST7789VI_FRCTRL1_NL_DOT     0x00 ///< Inversion: dot inversion
#define ST7789VI_FRCTRL1_NL_COL     0xE0 ///< Inversion: column inversion
#define ST7789VI_FRCTRL1_RTN_58     0x00 ///< Frame rate control: 58Hz
#define ST7789VI_FRCTRL1_RTN_57     0x01 ///< Frame rate control: 57Hz
#define ST7789VI_FRCTRL1_RTN_55     0x02 ///< Frame rate control: 55Hz
#define ST7789VI_FRCTRL1_RTN_53     0x03 ///< Frame rate control: 53Hz
#define ST7789VI_FRCTRL1_RTN_52     0x04 ///< Frame rate control: 52Hz
#define ST7789VI_FRCTRL1_RTN_50     0x05 ///< Frame rate control: 50Hz
#define ST7789VI_FRCTRL1_RTN_49     0x06 ///< Frame rate control: 49Hz
#define ST7789VI_FRCTRL1_RTN_48     0x07 ///< Frame rate control: 48Hz
#define ST7789VI_FRCTRL1_RTN_46     0x08 ///< Frame rate control: 46Hz
#define ST7789VI_FRCTRL1_RTN_45     0x09 ///< Frame rate control: 45Hz
#define ST7789VI_FRCTRL1_RTN_44     0x0A ///< Frame rate control: 44Hz
#define ST7789VI_FRCTRL1_RTN_43     0x0B ///< Frame rate control: 43Hz
#define ST7789VI_FRCTRL1_RTN_42     0x0C ///< Frame rate control: 42Hz
#define ST7789VI_FRCTRL1_RTN_41     0x0D ///< Frame rate control: 41Hz
#define ST7789VI_FRCTRL1_RTN_40     0x0E ///< Frame rate control: 40Hz
#define ST7789VI_FRCTRL1_RTN_39     0x0F ///< Frame rate control: 39Hz

// Parameters for ST7789VI_PARCTRL command
#define ST7789VI_PARCTRL_NDL        0x80 ///< Source output level selection in non-display area in partial mode ('0' = V63, '1' = V0)
#define ST7789VI_PARCTRL_PTGISC     0x10 ///< Non-display area scan mode ('0' = normal mode, '1' = interval scan mode)
// Non-display area scan frequency selection in interval scan mode
#define ST7789VI_PARCTRL_ISC_EVRY   0x00 ///< Every frame
#define ST7789VI_PARCTRL_ISC_3      0x01 ///< 1/3 frame
#define ST7789VI_PARCTRL_ISC_5      0x02 ///< 1/5 frame
#define ST7789VI_PARCTRL_ISC_7      0x03 ///< 1/7 frame
#define ST7789VI_PARCTRL_ISC_9      0x04 ///< 1/9 frame
#define ST7789VI_PARCTRL_ISC_11     0x05 ///< 1/11 frame
#define ST7789VI_PARCTRL_ISC_13     0x06 ///< 1/13 frame
#define ST7789VI_PARCTRL_ISC_15     0x07 ///< 1/15 frame
#define ST7789VI_PARCTRL_ISC_17     0x08 ///< 1/17 frame
#define ST7789VI_PARCTRL_ISC_19     0x09 ///< 1/19 frame
#define ST7789VI_PARCTRL_ISC_21     0x0A ///< 1/21 frame
#define ST7789VI_PARCTRL_ISC_23     0x0B ///< 1/23 frame
#define ST7789VI_PARCTRL_ISC_25     0x0C ///< 1/25 frame
#define ST7789VI_PARCTRL_ISC_27     0x0D ///< 1/27 frame
#define ST7789VI_PARCTRL_ISC_29     0x0E ///< 1/29 frame
#define ST7789VI_PARCTRL_ISC_31     0x0F ///< 1/31 frame

// Parameters for ST7789VI_GCTRL command
// VGH setting
#define ST7789VI_GCTRL_VGHS_1220    0x00 ///< 12.20V
#define ST7789VI_GCTRL_VGHS_1254    0x01 ///< 12.54V
#define ST7789VI_GCTRL_VGHS_1289    0x02 ///< 12.89V
#define ST7789VI_GCTRL_VGHS_1326    0x03 ///< 13.26V
#define ST7789VI_GCTRL_VGHS_1365    0x04 ///< 13.65V
#define ST7789VI_GCTRL_VGHS_1406    0x05 ///< 14.06V
#define ST7789VI_GCTRL_VGHS_1450    0x06 ///< 14.50V
#define ST7789VI_GCTRL_VGHS_1497    0x07 ///< 14.97V
// VGL setting
#define ST7789VI_GCTRL_VGLS_0716    0x00 ///< -7.16V
#define ST7789VI_GCTRL_VGLS_0767    0x01 ///< -7.67V
#define ST7789VI_GCTRL_VGLS_0823    0x02 ///< -8.23V
#define ST7789VI_GCTRL_VGLS_0887    0x03 ///< -8.87V
#define ST7789VI_GCTRL_VGLS_0960    0x04 ///< -9.60V
#define ST7789VI_GCTRL_VGLS_1043    0x05 ///< -10.43V
#define ST7789VI_GCTRL_VGLS_1138    0x06 ///< -11.38V
#define ST7789VI_GCTRL_VGLS_1250    0x07 ///< -12.50V

// Parameters for ST7789VI_DGMEN command
#define ST7789VI_DGMEN_DGMEN        0x04 ///< Digital gamma enable ('0' = disable, '1' = enable)

// Parameters for ST7789VI_LCMCTRL command
#define ST7789VI_LCMCTRL_XMY        0x40 ///< XOR MY setting in command 36h
#define ST7789VI_LCMCTRL_XBGR       0x20 ///< XOR RGB setting in command 36h
#define ST7789VI_LCMCTRL_XREV       0x10 ///< XOR inverse setting in command 21h
#define ST7789VI_LCMCTRL_XMH        0x08 ///< this bit can reverse source output order and only support for RGB interface without RAM mode
#define ST7789VI_LCMCTRL_XMV        0x04 ///< XOR MV setting in command 36h
#define ST7789VI_LCMCTRL_XMX        0x02 ///< XOR MX setting in command 36h
#define ST7789VI_LCMCTRL_XGS        0x01 ///< XOR GS setting in command E4h

// Parameters for ST7789VI_VDVVRHEN command
#define ST7789VI_VDVVRHEN_CMDEN     0x01 ///< VDV and VRH command write enable ('0' = values come from NVM, '1' = values come from command write)

// Parameters for ST7789VI_FRCTRL2 command
#define ST7789VI_FRCTRL2_NLA_DOT    0x00 ///< Inversion selection in normal mode: dot inversion
#define ST7789VI_FRCTRL2_NLA_COL    0x07 ///< Inversion selection in normal mode: column inversion

// Parameters for ST7789VI_CABCCTRL command
#define ST7789VI_CABCCTRL_LEDONREV  0x08 ///< Status of LED_ON ('0' = normal status, '1' = reversed status)
#define ST7789VI_CABCCTRL_DPOFPWM   0x04 ///< Initial state of LEDPWM ('0' = low, '1' = high)
#define ST7789VI_CABCCTRL_PWMFIX    0x02 ///< LEDPWM fix control ('0' = LEDPWM control by CABC, '1' = fix LEDPWM in “ON” status)
#define ST7789VI_CABCCTRL_PWMPOL    0x01 ///< LEDPWM polarity control ('0' = polarity high, '1' = polarity low)
