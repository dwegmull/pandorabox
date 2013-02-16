#ifndef VFD_H
#define VFD_H
// commands
#define VFD_CLEARDISPLAY 0x01
#define VFD_RETURNHOME 0x02
#define VFD_ENTRYMODESET 0x04
#define VFD_DISPLAYCONTROL 0x08
#define VFD_CURSORSHIFT 0x10
#define VFD_FUNCTIONSET 0x30
#define VFD_SETCGRAMADDR 0x40
#define VFD_SETDDRAMADDR 0x80

// flags for display entry mode
#define VFD_ENTRYRIGHT 0x00
#define VFD_ENTRYLEFT 0x02
#define VFD_ENTRYSHIFTINCREMENT 0x01
#define VFD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define VFD_DISPLAYON 0x04
#define VFD_DISPLAYOFF 0x00
#define VFD_CURSORON 0x02
#define VFD_CURSOROFF 0x00
#define VFD_BLINKON 0x01
#define VFD_BLINKOFF 0x00

// flags for display/cursor shift
#define VFD_DISPLAYMOVE 0x08
#define VFD_CURSORMOVE 0x00
#define VFD_MOVERIGHT 0x04
#define VFD_MOVELEFT 0x00

// flags for function set
#define VFD_2LINE 0x08
#define VFD_1LINE 0x00
#define VFD_BRIGHTNESS25 0x03
#define VFD_BRIGHTNESS50 0x02
#define VFD_BRIGHTNESS75 0x01
#define VFD_BRIGHTNESS100 0x00

#define VFD_SPICOMMAND 0xF8
#define VFD_SPIDATA 0xFA

#endif // VFD_H
