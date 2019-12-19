

#ifdef DO_NOT_USE_LIBC
#include "nolibc.h"
#ifndef NOLIBC
typedef   signed long       ssize_t;
typedef unsigned long        size_t;
static ssize_t write(int fd, const void *buf, size_t count);
#endif
#else
#include <unistd.h>
#endif


int main(void)
{
    write(1,"Hello world\n",13);
    return 0;
}

#ifndef NOLIBC

asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "pop %rdi\n"                // argc   (first arg, %rdi)
    "mov %rsp, %rsi\n"          // argv[] (second arg, %rsi)
    "lea 8(%rsi,%rdi,8),%rdx\n" // then a NULL then envp (third arg, %rdx)
    "and $-16, %rsp\n"          // x86 ABI : esp must be 16-byte aligned when
    "sub $8, %rsp\n"            // entering the callee
    "call main\n"               // main() returns the status code, we'll exit with it.
    "movzb %al, %rdi\n"         // retrieve exit code from 8 lower bits
    "mov $60, %rax\n"           // NR_exit == 60
    "syscall\n"                 // really exit
    "hlt\n"                     // ensure it does not return
    "");

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
    long _ret;                                                            \
    register long _num  asm("rax") = (num);                               \
    register long _arg1 asm("rdi") = (long)(arg1);                        \
    register long _arg2 asm("rsi") = (long)(arg2);                        \
    register long _arg3 asm("rdx") = (long)(arg3);                        \
                                          \
    asm volatile (                                                        \
        "syscall\n"                                                   \
        : "=a" (_ret)                                                 \
        : "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
          "0"(_num)                                                   \
        : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
    );                                                                    \
    _ret;                                                                 \
})


static ssize_t sys_write(int fd, const void *buf, size_t count)
{
    return my_syscall3(1, fd, buf, count);
}

static ssize_t write(int fd, const void *buf, size_t count)
{
    ssize_t ret = sys_write(fd, buf, count);

    if (ret < 0) {
        //SET_ERRNO(-ret);
        ret = -1;
    }
    return ret;
}

#endif  // #ifndef NOLIBC
