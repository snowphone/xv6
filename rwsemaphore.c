#include "rwsemaphore.h"
#include "defs.h"
#include "proc.h"

#define until(pred) while (!(pred))

void initrwsema(struct rwsemaphore *lk) {
  lk->refCount = 0;
  initlock(&lk->lk, myproc()->name);
  lk->writing = false;
  lk->head = 0;
}

void downreadsema(struct rwsemaphore *lk) {
  acquire(&lk->lk);

  until(!lk->writing) {
    lk->head = enqueue(lk->head, myproc());
    sleep(lk, &lk->lk);
  }
  ++lk->refCount;

  release(&lk->lk);
}

void upreadsema(struct rwsemaphore *lk) {
  acquire(&lk->lk);
  --lk->refCount;
  wakeup_one_proc(lk, &lk->head);
  release(&lk->lk);
}

void downwritesema(struct rwsemaphore *lk) {
  acquire(&lk->lk);
  until(!lk->writing) {
    lk->head = enqueue(lk->head, myproc());
    sleep(lk, &lk->lk);
  }
  lk->writing = true;
  until(lk->refCount == 0) {
    lk->head = cutInQueue(lk->head, myproc());
    sleep(lk, &lk->lk);
  }

  release(&lk->lk);
}

void upwritesema(struct rwsemaphore *lk) {
  acquire(&lk->lk);
  lk->writing = false;
  wakeup_one_proc(lk, &lk->head);
  release(&lk->lk);
}
