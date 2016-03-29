#include "petterson.h"

static int flag[N];
static int turn[N];

void lock(int i) {
    for (int count = 0; count < N - 1; ++count) {
        flag[i] = count + 1;
        turn[count] = i;

        bool found = true;
        while (turn[count] == i && found) {
            found = false;
            for (int k = 0; !fount && k != N; ++k) {
                if (k == i)
                    continue;
                found = flag[k] > count;
            }
        }
    }
}

void unlock(int i) {
    flag[i] = 0;
}
