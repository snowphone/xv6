#pragma once

struct semaphore {
};

void initsema(struct semaphore* lk, int count);

// Return the number of remaining threads that can access the section 
int downsema(struct semaphore* lk);

// Return the number of remaining threads that can access the section 
int upsema(struct semaphore* lk);
