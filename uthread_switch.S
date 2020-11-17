	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register; it is caller saved.
 */
	.globl thread_switch
thread_switch:
	pushal						// Backup registers
	movl current_thread, %eax
	movl %esp, (%eax)			// Backup stack

	movl next_thread, %eax
	movl %eax, current_thread	// Set current_thread = next_thread
	movl (%eax), %esp			// Restore stack
	xor %eax, %eax
	mov %eax, next_thread		// Set next_thread = 0
	popal						// Restore registers

	ret				/* pop return address from stack */
