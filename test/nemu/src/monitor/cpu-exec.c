#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 1000001

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

static vaddr_t old_eip;

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
	
  old_eip = cpu.eip;
	
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);

#ifdef DEBUG
    /* TODO: check watchpoints here. */

	WP* p = scan_watchpoint();
	if(p!=NULL){
		nemu_state = NEMU_STOP;
		
		printf("Hit watchpoint %d at address 0x%08x\n", p->NO, old_eip);
		printf("expr\t= %s\n", p->expr);
		printf("old value = 0x%08x\n", p->old_val);
		printf("new valie = 0x%08x\n", p->new_val);
		printf("program paused\n");
		p->old_val = p->new_val;
	}

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
