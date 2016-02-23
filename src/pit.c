#include "pit.h"
#include "ioport.h"

void set_up_pit(void) {
    out8(PIT_CONTROL, FIRST_COMMAND);
    out8(PIT_DATA, DIVIDER_LOW);
    out8(PIT_DATA, DIVIDER_HIGH);
}
