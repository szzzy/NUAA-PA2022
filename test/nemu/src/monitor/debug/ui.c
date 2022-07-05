#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}
static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	int count;
	if(arg == NULL) {	/*no numbers so only run once*/
		cpu_exec(1);
		return 0;
	}
	sscanf(arg, "%d", &count);
	cpu_exec(count);
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL, " ");
	if(strcmp(arg, "r") == 0) {
		for(int i=0;i<8;i++) {
			printf("%s\t0x%08x\t%d\t\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
		}	/*print the register informations*/
		printf("eip\t0x%08x\t%d\t\n", cpu.eip, cpu.eip);
	}
	else if (strcmp(arg, "w") == 0) {
		//printf("Waiting to add!!\n"); /*waiting to add*/
		list_watchpoint();
	}
	return 0;
}

static int cmd_x(char *args){
	char *arg = strtok(NULL, " ");
	char *arg_1 = strtok(NULL, " ");
	int count;
	vaddr_t address;
	sscanf(arg, "%d", &count);
	//sscanf(arg_1, "%x", &address);
	
	bool* success = false;
	address = expr(arg_1, success);
	
	printf("Address\t\tDword block\tByte sequence\n");
    for(int i=0; i<count;i++){
        printf("0x%08x\t0x%08x\t", address, vaddr_read(address, 4));
		for(int j=0;j<4;j++) {
			printf("%02x ", vaddr_read(address+j, 1));/*read 1 byte once*/
		}
		printf("\n");
		address += 4; /*address add 4 bytes to the next Dword block*/  
    }
	return 0;
}

static int cmd_p(char* args){
	bool* success=false;
	uint32_t result;
	result = expr(args,success);
	printf("%d\n", result);
	return 0;
}

static int cmd_w(char* args){
	char *arg = strtok(NULL, " ");
	WP* p = new_wp(arg);
	printf("Set watchpoint #%d\n", p->NO);
	printf("expr\t= %s\n", p->expr);
	printf("old value = %d\n", p->old_val);
	return 0;
}


static int cmd_d(){
	char *arg = strtok(NULL, " ");
	int num;
	sscanf(arg, "%d", &num);
	delete_watchpoint(num);
	return 0;
}


static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "One Step", cmd_si },
  { "info", "Display informations about all regisiters", cmd_info},
  { "x", "Scan memory", cmd_x},
  /* TODO: Add more commands */
  { "p", "Expression evaluation", cmd_p},
  { "w", "Set Watchpoint", cmd_w},
  { "d", "Delete Watchpoint", cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
