#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

static kernel_pid_t rcv_pid;
static char rcv_stack[THREAD_STACKSIZE_DEFAULT];
 
static void *rcv(void *arg)
{
    (void)arg;

    msg_t msg;

    while (1) {
        msg_receive(&msg);
        printf("Received %" PRIu32 "\n", msg.content.value);
    }

    return NULL;
}

static kernel_pid_t snd_pid;
static char snd_stack[THREAD_STACKSIZE_DEFAULT];
 
static void *snd(void *arg)
{
    (void)arg;

    msg_t msg;
 
    msg.content.value = 0;

    while(1) {
        msg_send(&msg, rcv_pid);
        msg.content.value++;
        xtimer_sleep(2);
    }

    return NULL;
}

static char tmr_stack[THREAD_STACKSIZE_MAIN];

static void *tmr(void *arg)
{
    while(1) {
        int now = xtimer_now_usec();
        printf("The time is now: %d\n", (now / 1000000) );
        xtimer_sleep(12);
    }

    (void)arg;
    return NULL;
}

int main(void)
{
    puts("This is mimod4");

    rcv_pid = thread_create(rcv_stack, sizeof(rcv_stack),
                            THREAD_PRIORITY_MAIN - 1, 0, rcv, NULL, "rcv");
    snd_pid = thread_create(snd_stack, sizeof(snd_stack),
                            THREAD_PRIORITY_MAIN - 1, 0, snd, NULL, "snd");

    thread_create(tmr_stack, sizeof(tmr_stack),
                            THREAD_PRIORITY_MAIN - 2, 0, tmr, NULL, "tmr");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
