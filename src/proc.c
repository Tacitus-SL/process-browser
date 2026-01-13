/**
 * @file proc.c
 * @brief Implementation of process data collection from the Linux /proc filesystem.
 */

#include "proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

// Max PID value to track history usually goes up to 32768, but 131072 is safe for modern systems.
static unsigned long long cpu_history[131072] = {0};
static unsigned long long prev_system_time = 0;

// --- HELPER FUNCTIONS ---

/**
 * @brief Reads the process name (command) from /proc/[pid]/comm.
 *
 * @param pid Process ID.
 * @param buffer Output buffer.
 * @param buf_size Size of the output buffer.
 */
static void read_process_name(pid_t pid, char *buffer, size_t buf_size) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *f = fopen(path, "r");
    if (f) {
        if (fgets(buffer, buf_size, f)) {
            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = 0;
        }
        fclose(f);
    } else {
        snprintf(buffer, buf_size, "unknown");
    }
}

/**
 * @brief Reads the Resident Set Size (RSS) memory from /proc/[pid]/status.
 *
 * @param pid Process ID.
 * @return Memory usage in Kilobytes.
 */
static long read_process_memory(pid_t pid) {
    char path[256];
    char line[256];
    long memory = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld", &memory);
            break;
        }
    }
    fclose(f);
    return memory;
}

/**
 * @brief Resolves the username owner of a process via /proc/[pid] stats.
 *
 * @param pid Process ID.
 * @param buffer Output buffer for the username.
 * @param buf_size Size of the buffer.
 */
static void read_process_user(pid_t pid, char *buffer, size_t buf_size) {
    char path[256];
    struct stat info;
    snprintf(path, sizeof(path), "/proc/%d", pid);
    if (stat(path, &info) == 0) {
        struct passwd *pw = getpwuid(info.st_uid);
        if (pw) {
            strncpy(buffer, pw->pw_name, buf_size - 1);
            buffer[buf_size - 1] = 0;
            return;
        }
    }
    // Fallback to UID if name not found
    snprintf(buffer, buf_size, "%d", info.st_uid);
}

/**
 * @brief Reads the total system CPU time from /proc/stat.
 *
 * Sums up user, nice, system, idle, iowait, irq, softirq, and steal times.
 *
 * @return Total system CPU ticks.
 */
static unsigned long long get_system_time() {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) {
        return 0;
    }
    char line[256];
    if (fgets(line, sizeof(line), f)) {
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        // Read the first line (aggregate cpu usage)
        if (sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) >= 4) {
            fclose(f);
            return user + nice + system + idle + iowait + irq + softirq + steal;
        }
    }
    fclose(f);
    return 0;
}

/**
 * @brief Reads the total CPU time spent by a specific process.
 *
 * Parses /proc/[pid]/stat to retrieve utime and stime.
 *
 * @param pid Process ID.
 * @return Total process CPU ticks (user + system).
 */
static unsigned long long get_process_time(pid_t pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
       return 0;
    }

    unsigned long long utime = 0, stime = 0;
    char buffer[1024];

    if (fgets(buffer, sizeof(buffer), f)) {
        // Find the end of the process name (last closing parenthesis)
        // to handle names with spaces like "(Web Content)".
        char *rpar = strrchr(buffer, ')');
        if (rpar) {
            // Data starts after the parenthesis
            char *token = rpar + 2;

            unsigned long long temp;
            char c_temp;
            int i_temp;

            // Parsing format based on `man proc`. utime is 14th, stime is 15th.
            sscanf(token, "%c %d %d %d %d %d %u %llu %llu %llu %llu %llu %llu",
                &c_temp, &i_temp, &i_temp, &i_temp, &i_temp, &i_temp,
                (unsigned*)&i_temp, &temp, &temp, &temp, &temp,
                &utime, &stime);
        }
    }
    fclose(f);
    return utime + stime;
}

// --- PUBLIC FUNCTIONS ---

void proc_list_init(proc_list_t *plist) {
    plist->count = 0;
    // Clear history on startup
    memset(cpu_history, 0, sizeof(cpu_history));
}

void proc_list_update(proc_list_t *plist) {
    DIR *dir;
    struct dirent *entry;

    // 1. Calculate system time NOW
    unsigned long long current_system_time = get_system_time();
    unsigned long long system_delta = 0;

    if (prev_system_time > 0) {
        system_delta = current_system_time - prev_system_time;
    }

    // Get number of cores to scale percentage (0-100% * cores)
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores < 1) num_cores = 1;

    dir = opendir("/proc");
    if (!dir){
        return;
    }

    plist->count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (plist->count >= MAX_PROCESSES) break;

        // Processes are directories with numeric names
        if (isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            proc_info_t *proc = &plist->list[plist->count];

            // Fill basic info
            proc->pid = pid;
            read_process_name(pid, proc->name, sizeof(proc->name));
            proc->memory = read_process_memory(pid);
            read_process_user(pid, proc->user, sizeof(proc->user));

            // --- CPU CALCULATION ---
            unsigned long long current_proc_time = get_process_time(pid);
            unsigned long long proc_delta = 0;

            // Check bounds for history array
            if (pid < 131072) {
                // If we have history for this PID and time is valid
                if (cpu_history[pid] > 0 && current_proc_time >= cpu_history[pid]) {
                    proc_delta = current_proc_time - cpu_history[pid];
                }
                // Save current time for the next update frame
                cpu_history[pid] = current_proc_time;
            }

            // Calculate percentage
            if (system_delta > 0) {
                // Formula: (Process Delta / System Delta) * 100 * Cores
                proc->cpu_usage = (float)proc_delta / (float)system_delta * 100.0 * num_cores;
            } else {
                proc->cpu_usage = 0.0;
            }
            // ------------------

            plist->count++;
        }
    }
    closedir(dir);

    // Save system time for the next update
    prev_system_time = current_system_time;
}

void proc_list_filter(const proc_list_t *src, proc_list_t *dest, const char *filter_str) {
    if (!filter_str || strlen(filter_str) == 0) {
        *dest = *src;
        return;
    }

    dest->count = 0;
    for (int i = 0; i < src->count; i++) {
        // Use strcasestr (non-standard GNU extension, enabled by _GNU_SOURCE)
        if (strcasestr(src->list[i].name, filter_str)) {
            dest->list[dest->count] = src->list[i];
            dest->count++;
        }
    }
}

int proc_kill_process(pid_t pid) {
    // Send SIGTERM (15) for graceful shutdown.
    // Could use SIGKILL (9) for forced termination.
    return kill(pid, SIGTERM);
}