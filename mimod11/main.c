#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "ztimer.h"

//57kiB

#define MAX_CHANNELS 128
#define MAX_CHANNEL_NAME 20

typedef struct {
    //int tag;
    char name[MAX_CHANNEL_NAME];
    kernel_pid_t receiver;
} channel_t;

static channel_t channels[MAX_CHANNELS];
static int num_channels = 0;

int create_channel(const char *name) {
    if (num_channels >= MAX_CHANNELS) {
        return -1;  // Max channels reached
    }

    strncpy(channels[num_channels].name, name, MAX_CHANNEL_NAME - 1);
    channels[num_channels].name[MAX_CHANNEL_NAME - 1] = '\0';
    channels[num_channels].receiver = KERNEL_PID_UNDEF;

    num_channels++;
    return num_channels - 1;
}

int find_channel(const char *name) {
    for (int i = 0; i < num_channels; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int receive_on_channel(const char *name) {
    int channel_id = find_channel(name);
    if (channel_id < 0) {
        return -1;  // Channel not found
    }
    channels[channel_id].receiver = thread_getpid();

    return channel_id;
}

int send_to_channel(const char *name, msg_t msg) {
    int channel_id = find_channel(name);
    if (channel_id < 0 || channels[channel_id].receiver == KERNEL_PID_UNDEF) {
        return -1;  // Channel not found or no receiver
    }

    return msg_send(&msg, channels[channel_id].receiver);
}

static kernel_pid_t proc1_pid;
static char proc1_stack[THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t proc2_pid;
static char proc2_stack[THREAD_STACKSIZE_DEFAULT];

static char tmr_stack[THREAD_STACKSIZE_MAIN];

static void *proc1(void *arg) {
    (void)arg;

    msg_t msg;

    int channel_id = receive_on_channel("my_channel");
    if (channel_id < 0) {
        puts("Failed to subscribe to channel");
        return NULL;
    }

    while (1) {
        msg_receive(&msg);
        printf("Recd by proc1 %" PRIu32 "\n", msg.content.value);
    }

    return NULL;
}

static void *proc2(void *arg) {
    (void)arg;

    msg_t msg;

    msg.content.value = 42;

    while(1) {
        send_to_channel("my_channel", msg);
        printf("Sent by proc2 %" PRIu32 "\n", msg.content.value);

        ztimer_sleep(ZTIMER_SEC, 1);
    }

    return NULL;
}

static void *tmr(void *arg) {
    (void)arg;

    while(1) {
        ztimer_sleep(ZTIMER_SEC, 10);
        printf("Uptime: %dsec\n", (int)ztimer_now(ZTIMER_SEC));
    }

    return NULL;
}

int main(void) {
    puts("This is mimod11");

    /* >CHAN  */
    int channel_one_id = create_channel("one");  
    printf("channel_one_id %d\n", channel_one_id);

    int proc1_chans = {channel_one_id};
    int proc1_chans_size = sizeof(proc1_chans);

    int proc2_chans = {channel_one_id};
    int proc2_chans_size = sizeof(proc2_chans);

    //iamerejh

    /* <CHAN  */


    /* >PAR */ 
    /* do not yield until all global proc_pids set */
    proc1_pid = thread_create(proc1_stack, sizeof(proc1_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_WOUT_YIELD, proc1, NULL, "proc1");

    proc2_pid = thread_create(proc2_stack, sizeof(proc2_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_WOUT_YIELD, proc2, NULL, "proc2");

    thread_yield();                       
    /* <PAR */

    thread_create(tmr_stack, sizeof(tmr_stack),
                            THREAD_PRIORITY_MAIN - 2, 0, tmr, NULL, "tmr");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

