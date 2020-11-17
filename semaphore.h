#pragma once
#include "types.h"
#include "spinlock.h"

struct semaphore {
  uint availableSeats;       // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  struct proc* head;
};

void initsema(struct semaphore* lk, int count);

// Return the number of remaining threads that can access the section 
int downsema(struct semaphore* lk);

// Return the number of remaining threads that can access the section 
int upsema(struct semaphore* lk);
