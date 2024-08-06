#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "ztimer.h"

//57kiB

static kernel_pid_t proc1_pid;
static char proc1_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t proc2_pid;
static char proc2_stack[THREAD_STACKSIZE_DEFAULT];

static char tmr_stack[THREAD_STACKSIZE_MAIN];

static void *proc1(void *arg)
{
    (void)arg;

    msg_t msg;

    while (1) {
        msg_receive(&msg);
        printf("Recd by proc1 %" PRIu32 "\n", msg.content.value);

        msg.content.value = msg.content.value * 2;

        msg_send(&msg, proc2_pid);
        printf("Sent by proc1 %" PRIu32 "\n", msg.content.value);
    }

    return NULL;
}

static void *proc2(void *arg)
{
    (void)arg;

    msg_t msg;

    msg.content.value = 0;

    while(1) {
        msg_send(&msg, proc1_pid);
        printf("Sent by proc2 %" PRIu32 "\n", msg.content.value);

        msg_receive(&msg);
        printf("Recd by proc2 %" PRIu32 "\n", msg.content.value);

        msg.content.value++;
        ztimer_sleep(ZTIMER_SEC, 1);
    }

    return NULL;
}

static void *tmr(void *arg)
{
    (void)arg;

    while(1) {
        ztimer_sleep(ZTIMER_SEC, 10);
        printf("Uptime: %dsec\n", (int)ztimer_now(ZTIMER_SEC));
    }

    return NULL;
}

int main(void)
{
    puts("This is mimod10");

    /* do not yield until all global proc_pids set */
    proc1_pid = thread_create(proc1_stack, sizeof(proc1_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_WOUT_YIELD, proc1, NULL, "proc1");

    proc2_pid = thread_create(proc2_stack, sizeof(proc2_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_WOUT_YIELD, proc2, NULL, "proc2");

    thread_yield();                       

    thread_create(tmr_stack, sizeof(tmr_stack),
                            THREAD_PRIORITY_MAIN - 2, 0, tmr, NULL, "tmr");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

