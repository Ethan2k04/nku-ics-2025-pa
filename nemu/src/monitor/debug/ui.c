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

static int cmd_help(char *args);

static int cmd_si(char *args) {
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }
  char * steps = strtok(NULL, " ");
  if (steps == NULL) {
    cpu_exec(1);
  } else {
    int n = 1;
    if (sscanf(steps, "%d", &n) == 1 && n > 0) { cpu_exec(n); }
    else { printf("Invalid number: %s.\n", steps); }
  }
  return 0;
}

static int cmd_info(char *args) {
  char * arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Please enter r or w.\n");
    return 0;
  }
  if (strcmp(arg, "r") == 0) {
    /* TODO✔️: Print registers */
    reg_display();
  }
  else if (strcmp(arg, "w") == 0) {
    /* TODO✔️: List all the break points */
    watchpoint_display();
  }
  else { printf("Invalid command: %s.\n", arg); }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    /* TODO: Error handler */
    printf("No expression.\n");
    TODO();
    return 0;
  }
  bool success = true;
  uint32_t result = expr(args, &success);
  if (success) { printf("Result: %u\n", result); }
  else { printf("Invalid expression: %s.\n", args); }
  return 0;
}

static int cmd_x(char *args) {
  char * n_str = strtok(NULL, " ");
  char * s_expr = strtok(NULL, " ");
  if (n_str == NULL || s_expr == NULL) {
    printf("Invalid arguments.\n");
    return 0;
  }
  int n = 0;
  if (sscanf(n_str, "%d", &n) != 1 || n <= 0) {
    printf("Invalid number: %s.\n", n_str);
    return 0;
  }
  bool success = true;
  uint32_t addr = expr(s_expr, &success);
  if (!success) {
    printf("Invalid expression: %s.\n", s_expr);
    return 0;
  }
  for (int i = 0; i < n; i++) {
    printf("0x%08x: ", addr);
    for (int j = 0; j < 4; j++) {
      printf("0x%02x ", vaddr_read(addr, 1));
      addr++;
    }
    printf("\n");
  }
  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) {
		puts("Command format: \"w EXPR\"");
		return 0;
	}
	args += strspn(args, " ");
	int no = set_watchpoint(args);
	if (no == -1) {
		printf("invalid expression: '%s'\n", args);
		return 0;
	}
	printf("set watchpoint %d\n", no);
	return 0;
}

static int cmd_d(char *args) {
  char * p = strtok(NULL, " ");
  if (p == NULL) {
    printf("Invalid arguments.\n");
    /* TODO: Error handler */
    TODO();
    return 0;
  }
  int no;
  for (; p != NULL; p = strtok(NULL, " ")) {
		if (sscanf(p, "%d", &no) != 1) {
			printf("Bad breakpoint number: '%s'\n", p);
			return 0;
		}
		delete_breakpoint(no);
	}
	return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO✔️: Add more commands */
  { "si", "Single step execution", cmd_si },
  { "info", "Display information of regs/watchpoint", cmd_info},
  { "p", "Evaluate expression", cmd_p},
  { "x", "Scan memory", cmd_x},
  { "w", "Set watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d},
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
