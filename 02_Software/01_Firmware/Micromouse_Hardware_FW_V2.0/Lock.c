#include "lock.h"
#include "cmsis_gcc.h"   // oder core_cmX.h

lock_key_t lock_acquire(void) {
    lock_key_t key = __get_PRIMASK();
    __disable_irq();
    return key;
}

void lock_release(lock_key_t key) {
    __set_PRIMASK(key);
}
