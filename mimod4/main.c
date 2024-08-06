#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

char stack[THREAD_STACKSIZE_MAIN];

static kernel_pid_t rcv_pid;
static char rcv_stack[THREAD_STACKSIZE_DEFAULT];
 
static void *rcv(void *arg)
{
    msg_t msg_req, msg_resp;
 
    (void)arg;

    while (1) {
        msg_receive(&msg_req);
        msg_resp.content.value = msg_req.content.value + 1;
        msg_reply(&msg_req, &msg_resp);
    }   

    return NULL;
}

static kernel_pid_t snd_pid;
static char snd_stack[THREAD_STACKSIZE_DEFAULT];
 
static void *snd(void *arg)
{
    msg_t msg_req, msg_snd;
 
    (void)arg;

    msg_snd.content.value = 0;

    while(1) {
        msg_req.content.value = msg_snd.content.value;
        msg_send_receive(&msg_req, &msg_snd, rcv_pid);
        printf("Result: %" PRIu32 "\n", msg_snd.content.value);
        xtimer_sleep(2);
    }

    return NULL;
}

void *thread_handler(void *arg)
{
    while(1) {
        int now = xtimer_now_usec();
        printf("The time is now: %d\n", now);
        xtimer_sleep(12);
    }

    (void)arg;
    return NULL;
}

int main(void)
{
    puts("This is Task-04");

    rcv_pid = thread_create(rcv_stack, sizeof(rcv_stack),
                            THREAD_PRIORITY_MAIN - 1, 0, rcv, NULL, "rcv");
    snd_pid = thread_create(snd_stack, sizeof(snd_stack),
                            THREAD_PRIORITY_MAIN - 3, 0, snd, NULL, "snd");

    thread_create(stack, sizeof(stack),
                  THREAD_PRIORITY_MAIN - 2,
                  THREAD_CREATE_STACKTEST,
                  thread_handler, NULL,
                  "thread");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
