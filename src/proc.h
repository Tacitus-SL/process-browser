#ifndef PROC_H
#define PROC_H

#include <sys/types.h>
#include <signal.h>

/**
 * @brief Maximum number of processes that can be stored in the list.
 */
#define MAX_PROCESSES 2048

/**
 * @brief Structure representing a single process information.
 */
typedef struct {
    pid_t pid;                  /**< Process ID */
    char name[256];             /**< Process command name */
    char user[32];              /**< Name of the user who owns the process */
    long memory;                /**< Resident Set Size (RSS) memory usage in Kilobytes */
    float cpu_usage;            /**< CPU usage percentage (0.0 to 100.0 * cores) */
} proc_info_t;

/**
 * @brief Container structure for a list of processes.
 */
typedef struct {
    proc_info_t list[MAX_PROCESSES]; /**< Array of process structures */
    int count;                       /**< Current number of processes in the list */
} proc_list_t;

/**
 * @brief Initializes the process list structure.
 *
 * Resets the count to zero and clears internal history buffers.
 *
 * @param plist Pointer to the process list to initialize.
 */
void proc_list_init(proc_list_t *plist);

/**
 * @brief Updates the process list by reading the system /proc directory.
 *
 * Scans /proc for running processes, calculates CPU usage since the last update,
 * and populates the list with current data.
 *
 * @param plist Pointer to the process list to update.
 */
void proc_list_update(proc_list_t *plist);

/**
 * @brief Filters the process list based on a search string.
 *
 * Performs a case-insensitive search. If the filter string matches a process name,
 * that process is copied to the destination list.
 *
 * @param src Pointer to the source list (all processes).
 * @param dest Pointer to the destination list (filtered processes).
 * @param filter_str The string to search for. If NULL or empty, copies all processes.
 */
void proc_list_filter(const proc_list_t *src, proc_list_t *dest, const char *filter_str);

/**
 * @brief Sends a termination signal to a process.
 *
 * Uses SIGTERM to attempt a graceful shutdown of the process.
 *
 * @param pid The Process ID to kill.
 * @return 0 on success, -1 on failure (errno is set).
 */
int proc_kill_process(pid_t pid);

#endif // PROC_H