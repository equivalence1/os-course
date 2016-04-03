#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "thread.h"
#include "interrupt.h"

void schedule(void);
void yield(void);
void join(int pid);

#endif // __SCHEDULER_H__
