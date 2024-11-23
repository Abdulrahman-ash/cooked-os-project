#include "headers.h"
#include <string.h>
#define MAX_PROCESSES 100

// Define the process structure
typedef struct Process {
    int id;
    int arrival_time;
    int runtime;
    int priority;
} Process;

void clearResources(int);

// Reads processes from the input file
void read_processes(const char* filename, Process processes[], int* count);

int main(int argc, char* argv[]) {
    signal(SIGINT, clearResources); // Handle Ctrl+C to clean resources

    Process processes[MAX_PROCESSES];
    int process_count = 0;

    // Read the input file
    read_processes("processes.txt", processes, &process_count);

    // Ask the user for the chosen scheduling algorithm and its parameters
    printf("Select a scheduling algorithm:\n");
    printf("1. Shortest Job First (SJF)\n");
    printf("2. Preemptive Highest Priority First (PHPF)\n");
    printf("3. Round Robin (RR)\n");
    int algorithm_choice;
    scanf("%d", &algorithm_choice);

    // For Round Robin, ask for the time quantum
    int time_quantum = 0;
    if (algorithm_choice == 3) {
        printf("Enter the time quantum for Round Robin: ");
        scanf("%d", &time_quantum);
    }

    // Create scheduler and clock processes
    pid_t scheduler_pid = fork();
    if (scheduler_pid == -1) {
        perror("Error creating Scheduler process");
        exit(EXIT_FAILURE);
    }
    if (scheduler_pid == 0) {
        execl("./scheduler", "scheduler", NULL);
        perror("Error executing Scheduler");
        exit(EXIT_FAILURE);
    }

    pid_t clock_pid = fork();
    if (clock_pid == -1) {
        perror("Error creating Clock process");
        exit(EXIT_FAILURE);
    }
    if (clock_pid == 0) {
        execl("./clk", "clk", NULL);
        perror("Error executing Clock");
        exit(EXIT_FAILURE);
    }

    // Initialize the clock
    initClk();

    // Create a message queue
    key_t key = ftok("scheduler", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }

    // Main loop to send process data to the scheduler
    for (int i = 0; i < process_count; i++) {
        // Wait for the process's arrival time
        while (getClk() < processes[i].arrival_time) {
            sleep(1);
        }

        // Prepare the message to send to the scheduler
        struct msg_buffer message;
        message.message_type = 1; // Fixed the field name
        message.process.id = processes[i].id;
        message.process.arrival_time = processes[i].arrival_time;
        message.process.runtime = processes[i].runtime;
        message.process.remaining_time = processes[i].runtime;
        message.process.priority = processes[i].priority;
        strcpy(message.process.state, "Ready");

        // Send the message
        if (msgsnd(msgid, &message, sizeof(message.process), 0) == -1) {
            perror("Error sending message to Scheduler");
            exit(EXIT_FAILURE);
        }

        printf("Process Generator: Sent process %d to Scheduler at time %d\n",
               processes[i].id, getClk());
    }

    // Wait for the scheduler and clock processes to finish
    waitpid(scheduler_pid, NULL, 0);
    waitpid(clock_pid, NULL, 0);

    // Clear clock resources
    destroyClk(true);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}

void clearResources(int signum) {
    destroyClk(true);
    printf("Process Generator: Resources cleared and exiting.\n");
    exit(0);
}

void read_processes(const char* filename, Process processes[], int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening processes.txt");
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip the header line

    *count = 0;
    while (fscanf(file, "%d\t%d\t%d\t%d", &processes[*count].id,
        &processes[*count].arrival_time,
        &processes[*count].runtime,
        &processes[*count].priority) != EOF) {
        (*count)++;
        }
        fclose(file);
}