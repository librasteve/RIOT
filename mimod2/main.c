#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

#define HELPER_THREAD_STACKSIZE  (THREAD_STACKSIZE_DEFAULT)
#define HELPER_THREAD_PRIORITY   (THREAD_PRIORITY_MAIN - 1)
#define MSG_QUEUE_SIZE           (8)

// Message type for communication
typedef enum {
    MSG_TYPE_SEND,
    MSG_TYPE_RECEIVE
} msg_type_t;

// Message structure
typedef struct {
    msg_t msg;
    msg_type_t type;
} my_msg_t;

// Helper thread function
void *helper_thread(void *arg) {
    (void)arg;
    msg_t msg_queue[MSG_QUEUE_SIZE];
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);

    while (1) {
        msg_t msg;
        if (msg_receive(&msg) < 0) {
            printf("Error receiving message\n");
            continue;
        }

        if (msg.type == MSG_TYPE_SEND) {
            printf("Helper: Received message from main thread\n");
        }
    }

    return NULL;
}

int main(void) {
    // Message queue for the main thread
    msg_t msg_queue[MSG_QUEUE_SIZE];
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);

    // Start the helper thread
    char helper_stack[HELPER_THREAD_STACKSIZE];
    thread_create(helper_stack, sizeof(helper_stack), HELPER_THREAD_PRIORITY,
                  THREAD_CREATE_STACKTEST, helper_thread, NULL, "helper");

    // Send a message to the helper thread
    my_msg_t my_msg;
    my_msg.msg.type = MSG_TYPE_SEND;
    msg_send(&my_msg.msg, thread_getpid());

    while (1) {
        // Main thread can perform other tasks here
        printf("Main thread is running\n");
        xtimer_sleep(2 * US_PER_SEC);
    }

    return 0;
}

