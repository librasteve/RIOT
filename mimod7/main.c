#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

//53kiB

static kernel_pid_t proc1_pid;
static char proc1_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t proc2_pid;
static char proc2_stack[THREAD_STACKSIZE_DEFAULT];

static char tmr_stack[THREAD_STACKSIZE_MAIN];

static void *proc1(void *arg)
{
    (void)arg;

    msg_t msg_out, msg_bak;

    while (1) {
        msg_receive(&msg_out);
        printf("Received by proc1 %" PRIu32 "\n", msg_out.content.value);

        msg_bak.content.value = msg_out.content.value * 2;
        msg_send(&msg_bak, proc2_pid);
        printf("Sent by proc1 %" PRIu32 "\n", msg_bak.content.value);
        sched_run();
    }

    return NULL;
}

static void *proc2(void *arg)
{
    (void)arg;

    msg_t msg_out, msg_bak;

    msg_out.content.value = 0;

    while(1) {
        msg_send(&msg_out, proc1_pid);
        printf("Sent by proc2 %" PRIu32 "\n", msg_out.content.value);
        sched_run();
        msg_out.content.value++;

        msg_receive(&msg_bak);
        printf("Received by proc2 %" PRIu32 "\n", msg_bak.content.value);

        xtimer_sleep(2);
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
                            THREAD_PRIORITY_MAIN - 1, 0, proc1, NULL, "proc1");
    proc2_pid = thread_create(proc2_stack, sizeof(proc2_stack),
                            THREAD_PRIORITY_MAIN - 3, 0, proc2, NULL, "proc2");

    thread_create(tmr_stack, sizeof(tmr_stack),
                            THREAD_PRIORITY_MAIN - 2, 0, tmr, NULL, "tmr");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

