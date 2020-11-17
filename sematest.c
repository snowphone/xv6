#include "types.h"
#include "user.h"
#include "fcntl.h"

int 
main()
{
	enum {INIT, DOWN, UP};
	int i, pid;

	// initialize the semaphore
	sematest(INIT); // Capacity = 5

	for (i = 0; i < 10; i++) {
		pid = fork();

		if (!pid)  // child
			break;
	}
	
	if (pid) { // parent
		sleep(300);
		for (i = 0; i < 10; i++) 
			wait();
		
		sematest(DOWN);
		printf(1, "Final %d\n", sematest(UP));
	} 
	else { // child
		printf(1, "%d Down : %d\n", i, sematest(DOWN));
		sleep(100);	
		printf(1, "%d   Up : %d\n", i, sematest(UP));
	}
	
	exit();
}
