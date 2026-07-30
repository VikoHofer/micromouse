#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

/* Typ für Lock-Key */
typedef uint32_t lock_key_t;

/* Lock-Funktionen */
lock_key_t lock_acquire(void);
void lock_release(lock_key_t key);

#endif
