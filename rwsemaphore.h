#pragma once

#include "types.h"
#include "spinlock.h"
#include <stdbool.h>

struct rwsemaphore {
	uint refCount;
	struct spinlock lk;
	bool writing;
	struct proc* head;
};

void initrwsema(struct rwsemaphore* lk);

void downreadsema(struct rwsemaphore *lk);
void upreadsema(struct rwsemaphore *lk);

void downwritesema(struct rwsemaphore *lk);
void upwritesema(struct rwsemaphore *lk);
