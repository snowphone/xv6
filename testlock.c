#include "types.h"
#include "user.h"
#include "fcntl.h"

int main() {
	int i, pid;
	
	testlock();

	for(i = 0; i < 10; ++i) {
		pid = fork();
		if(pid) {
			printf(1, "process %d is created\n", i);
			sleep(10);
		}
		else
			break;
	}

	if (pid) {
		sleep(100);
		testlock();
		for(i = 0; i < 10; ++i) {
			wait();
		}
	} else {
		// When testlock is called, it first checks whether it is holding a lock.
		// Since child's pid is different from the parent, the chlid process is not holding the lock.
		// Thus, the child tries to acquire the lock, but it cannot 
		// due to the fact that the same lock is actually held by the parent.
		// Consequently, what children can do is just go to sleep and wait for the parent's wakeup
		testlock();
		printf(1, "%d have acquired lock\n", i);
		testlock();
	}

	exit();
}
