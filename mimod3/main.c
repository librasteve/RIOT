#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
 
/*
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
 
int main(void)
{
    msg_t msg_req, msg_resp;
 
    msg_resp.content.value = 0;
    rcv_pid = thread_create(rcv_stack, sizeof(rcv_stack),
                            THREAD_PRIORITY_MAIN - 1, 0, rcv, NULL, "rcv");
    while (1) {
        msg_req.content.value = msg_resp.content.value;
        msg_send_receive(&msg_req, &msg_resp, rcv_pid);
        printf("Result: %" PRIu32 "\n", msg_resp.content.value);
    }
    return 0;
}
*/




char stack[THREAD_STACKSIZE_MAIN];

void *thread_handler(void *arg)
{
    /* ... */
    while(1) {
        int now = xtimer_now_usec();
        printf("The time is now: %d\n", now);
        xtimer_sleep(2);
    }

    (void)arg;
    return NULL;
}

int main(void)
{
    puts("This is Task-04");

    thread_create(stack, sizeof(stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  thread_handler, NULL,
                  "thread");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
