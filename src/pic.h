#ifndef __PIC_H__
#define __PIC_H__

#define MASTER_PIC_COMMAND             0x20
#define MASTER_PIC_DATA                0x21
#define SLAVE_PIC_COMMAND              0xA0
#define SLAVE_PIC_DATA                 0xA1

#define FIRST_WORD                     0x11 // 0b00010001 
#define ICW2_MASTER                    0x20
#define ICW2_SLAVE                     0x28
#define CONFIG_WORD_MASTER             0x04 // 0b00000100
#define CONFIG_WORD_SLAVE              0x02 // 0b00000010
#define MODE_WORD                      0x01 // 0b00000001
#define PIC_EOI                        0x20

#include "ioport.h"

void set_up_pic(void);
void pic_send_eoi(unsigned char);

#endif // __PIC_H__
