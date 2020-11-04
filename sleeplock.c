// Sleeping locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->head = 0;
}

// Push an item at the end of the linked list.
static struct proc* 
enqueue(struct proc* head, struct proc* item) {
	item->next = 0;
	if(!head)
		return item;
	for(struct proc* it = head; it; it = it->next) {
		if(!it->next) {
			it->next = item;
			break;
		}
	}
	return head;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  struct proc* me = myproc();
  while (lk->locked) {
    lk->head = enqueue(lk->head, me);
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = me->pid;
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup_one_proc(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



