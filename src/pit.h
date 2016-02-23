#ifndef __PIT_H__
#define __PIT_H__

#define PIT_CONTROL     0x43
#define PIT_DATA        0x40
#define FIRST_COMMAND   0x34 // 0b00110100
#define DIVIDER_LOW     0xFF
#define DIVIDER_HIGH    0xFF

void set_up_pit(void);

#endif // __PIT_H__
