// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display backtrace information", mon_backtrace},
	{ "showmappings", "Display mapping information", mon_showmappings}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

//[LAB2]
int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("Show mappings\n\n");
	if (!argv[1] || !argv[2])
	{
		cprintf("Usage: showmappings start_addr end_addr\n");
		return 1;
	}

	char* end_ptr = argv[1] + strlen(argv[1]) + 1;
	uintptr_t curr_addr = (uintptr_t) strtol(argv[1], &end_ptr, 16);
	end_ptr = argv[2] + strlen(argv[2]) + 1;
	uintptr_t end_addr = (uintptr_t) strtol(argv[2], &end_ptr, 16);
	cprintf("Start address: %08x\nEnd address: %08x\n", curr_addr, end_addr);

	while(curr_addr <= end_addr)
	{
		pte_t* pte = pgdir_walk(kern_pgdir, (void*)curr_addr, false);
		assert(kern_pgdir);
		if(!pte || !(*pte & PTE_P))
		{
			cprintf("virtual address 0x%08x is not mapped", curr_addr);
		}
		else
		{
			cprintf("virtual page 0x%08x -> physical page 0x%08x ", curr_addr, PTE_ADDR(*pte));
			cprintf("PTE_P: %d PTE_U: %d PTE_W: %d", *pte & PTE_P, *pte & PTE_U, *pte & PTE_W);
		}
		cprintf("\n");
		curr_addr += PGSIZE;
	}
	return 0;
}
//[LAB2]

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	struct Eipdebuginfo info;
	cprintf("Stack backtrace:\n");
	uint32_t* ebp = (uint32_t*)read_ebp();
	while (ebp != 0x0) 
	{
		uint32_t* eip = (uint32_t*)*(ebp + 1);
		// on success returns 0
		debuginfo_eip((uintptr_t)eip, &info);

		uint32_t i = 0;
		uint32_t arg[5];
		uint32_t* arg_iterator = ebp + 2;
		for(; i < 5; i++, arg_iterator++)
		{
			arg[i] = (uint32_t)*arg_iterator;
		}
		cprintf("ebp %08x eip %x args %08x %08x %08x %08x\n", ebp, eip, arg[0], arg[1], arg[2], arg[3]);
		cprintf("	%s:%d:", info.eip_file, info.eip_line);
		cprintf(" %.*s", info.eip_fn_namelen, info.eip_fn_name);
		cprintf("+%d\n", eip - info.eip_fn_addr);
			
		ebp = (uint32_t*)*ebp;
	}
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
