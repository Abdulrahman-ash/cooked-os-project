#include "headers.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

int main(int argc, char *argv[]) {
    initClk();

    // Create message queue to receive processes
    key_t key = ftok("scheduler", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }

    struct msg_buffer message;
    PCB processes[MAX_PROCESSES];
    int process_count = 0;
    PCB *running_process = NULL;
    int time_quantum = 2; // Default time quantum for Round Robin

    // Ask the user for the scheduling algorithm
    printf("Select a scheduling algorithm:\n");
    printf("1. Shortest Job First (SJF)\n");
    printf("2. Preemptive Highest Priority First (PHPF)\n");
    printf("3. Round Robin (RR)\n");
    int algorithm_choice;
    scanf("%d", &algorithm_choice);

    while (1) {
        int current_time = getClk();

        // Receive processes from Process Generator
        if (msgrcv(msgid, &message, sizeof(message.process), 1, IPC_NOWAIT) != -1) {
            PCB new_process = {
                .id = message.process.id,
                .arrival_time = message.process.arrival_time,
                .runtime = message.process.runtime,
                .remaining_time = message.process.runtime,
                .priority = message.process.priority
            };
            strcpy(new_process.state, "Ready");
            processes[process_count++] = new_process;

            printf("Scheduler: Received process %d at time %d\n", new_process.id, current_time);
        }

        switch (algorithm_choice) {
            case 1: // Shortest Job First (SJF)
                // Scheduling logic for SJF
                break;

            case 2: // Preemptive Highest Priority First (PHPF)
                // Scheduling logic for PHPF
                break;

            case 3: // Round Robin (RR)
                // Scheduling logic for RR
                break;

            default:
                printf("Invalid scheduling algorithm choice.\n");
                exit(EXIT_FAILURE);
        }

        // Check if all processes are finished
        int all_finished = 1;
        for (int i = 0; i < process_count; i++) {
            if (strcmp(processes[i].state, "Finished") != 0) {
                all_finished = 0;
                break;
            }
        }
        if (all_finished) break;

        sleep(1); // Simulate scheduler working every second
    }

    // Cleanup resources
    destroyClk(true);
    msgctl(msgid, IPC_RMID, NULL);
    printf("Scheduler: All processes completed. Exiting.\n");
    return 0;
}