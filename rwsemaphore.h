#pragma once

#include "types.h"
#include "spinlock.h"

struct rwsemaphore {
};

void initrwsema(struct rwsemaphore* lk);

void downreadsema(struct rwsemaphore *lk);
void upreadsema(struct rwsemaphore *lk);

void downwritesema(struct rwsemaphore *lk);
void upwritesema(struct rwsemaphore *lk);
