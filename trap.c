#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

#include <stdbool.h>
#include <stddef.h>

#define min(x,y) ((x) < (y) ? (x) : (y))

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

#define WITHIN(range, value) (range[0] <= (value) && (value) < range[1])
bool bind_page(uint fault_addr) {
  if (fault_addr >= KERNBASE) {
    return false;
  }

  struct proc *pr = myproc();
  struct proghdr ph = pr->ph;
  pde_t *pgdir = pr->pgdir;
  struct inode *inode = NULL;

  uint aligned_begin = PGROUNDDOWN(fault_addr);
  uint text_sect[2] = {ph.vaddr, ph.vaddr + ph.filesz};

  if (WITHIN(text_sect, aligned_begin)) {
    uint offset = ph.off + (aligned_begin - ph.vaddr);
    uint sz = min(PGSIZE, text_sect[1] - aligned_begin);

    if (allocuvm(pgdir, aligned_begin, aligned_begin + sz) == 0)
      goto bad;
    inode = namei((char *)pr->path);
    ilock(inode);
    if (loaduvm(pgdir, (void *)aligned_begin, inode, offset, sz) < 0)
      goto bad;
    iunlockput(inode);
    inode = NULL;

  } else { // bss/data/heap section
    if (allocuvm(pgdir, aligned_begin, aligned_begin + PGSIZE) == 0)
      return false;
  }
  return true;


bad:
	deallocuvm(pgdir, aligned_begin + PGSIZE, aligned_begin);
	if(inode)
		iunlockput(inode);
	return false;
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  uint page_fault_addr = rcr2();

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  case T_PGFLT:
    if (bind_page(page_fault_addr)) {
      break;
    }
        /* else: fall-through */

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
