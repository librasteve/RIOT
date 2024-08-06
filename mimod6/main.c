#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

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
        printf("Received by proc1 %" PRIu32 "\n", msg.content.value);
        xtimer_sleep(2);
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
        msg.content.value++;
    }

    return NULL;

}

static void *tmr(void *arg)
{
    while(1) {
        int now = xtimer_now_usec();
        printf("Uptime: %ds\n", (now / 1000000) );
        xtimer_sleep(10);
    }

    (void)arg;
    return NULL;
}

int main(void)
{
    puts("This is mimod7");

    proc1_pid = thread_create(proc1_stack, sizeof(proc1_stack),
                            THREAD_PRIORITY_MAIN - 3, 0, proc1, NULL, "proc1");
    proc2_pid = thread_create(proc2_stack, sizeof(proc2_stack),
                            THREAD_PRIORITY_MAIN - 1, 0, proc2, NULL, "proc2");

    thread_create(tmr_stack, sizeof(tmr_stack),
                            THREAD_PRIORITY_MAIN - 2, 0, tmr, NULL, "tmr");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

