#include "semaphore.h"
#include "types.h"
#include "proc.h"
#include "defs.h"

void initsema(struct semaphore* lk, int count) {
	lk->availableSeats = count;
	initlock(&lk->lk, myproc()->name);
	lk->head = 0;
}

// Return the number of remaining threads that can access the section 
int downsema(struct semaphore* lk) {
	int seats;

	acquire(&lk->lk);
	struct proc* me = myproc();
	while (!lk->availableSeats) {
		lk->head = enqueue(lk->head, me);
		sleep(lk, &lk->lk);
	}
	seats = --lk->availableSeats;
	release(&lk->lk);

	return seats;
}

// Return the number of remaining threads that can access the section 
int upsema(struct semaphore* lk) {
  int seats;

  acquire(&lk->lk);
  seats = ++lk->availableSeats;
  wakeup_one_proc(lk, &lk->head);
  release(&lk->lk);

  return seats;
}
