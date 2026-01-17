/**
 * @file proc.c
 * @brief Implementation of process data collection from /proc filesystem.
 */

#define _GNU_SOURCE
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

/*
 * Max PID value to track history. Usually goes up to 32768,
 * but 131072 is safe for modern systems.
 */
static unsigned long long cpu_history[131072] = {0};
static unsigned long long prev_system_time = 0;

/* HELPER FUNCTIONS */

/**
 * @brief Read process name from /proc/[pid]/comm.
 *
 * @param pid Process ID.
 * @param buffer Output buffer.
 * @param buf_size Size of output buffer.
 */
static void read_process_name(pid_t pid, char *buffer, size_t buf_size) {
	char path[256];
	snprintf(path, sizeof(path), "/proc/%d/comm", pid);
	FILE *f = fopen(path, "r");

	if (f) {
		if (fgets(buffer, buf_size, f)) {
			/* Remove trailing newline */
			buffer[strcspn(buffer, "\n")] = 0;
		}
		fclose(f);
	} else {
		snprintf(buffer, buf_size, "unknown");
	}
}

/**
 * @brief Read Resident Set Size memory from /proc/[pid]/status.
 *
 * @param pid Process ID.
 * @return Memory usage in kilobytes.
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
 * @brief Resolve username owner of process via /proc/[pid] stats.
 *
 * Falls back to numeric UID if username lookup fails.
 *
 * @param pid Process ID.
 * @param buffer Output buffer for username.
 * @param buf_size Size of buffer.
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
	/* Fallback to UID if name not found */
	snprintf(buffer, buf_size, "%d", info.st_uid);
}

/**
 * @brief Read total system CPU time from /proc/stat.
 *
 * Sums user, nice, system, idle, iowait, irq, softirq, and steal times.
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
		unsigned long long user, nice, system, idle;
		unsigned long long iowait, irq, softirq, steal;

		/* Read first line (aggregate CPU usage) */
		if (sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu "
			   "%llu %llu", &user, &nice, &system, &idle,
			   &iowait, &irq, &softirq, &steal) >= 4) {
			fclose(f);
			return user + nice + system + idle + iowait +
			       irq + softirq + steal;
		}
	}
	fclose(f);
	return 0;
}

/**
 * @brief Read total CPU time spent by specific process.
 *
 * Parses /proc/[pid]/stat to retrieve utime and stime.
 * Handles process names with spaces correctly.
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
		/*
		 * Find end of process name (last closing parenthesis)
		 * to handle names with spaces like "(Web Content)".
		 */
		char *rpar = strrchr(buffer, ')');
		if (rpar) {
			/* Data starts after the parenthesis */
			char *token = rpar + 2;
			unsigned long long temp;
			char c_temp;
			int i_temp;

			/*
			 * Parsing format based on `man proc`.
			 * utime is 14th, stime is 15th.
			 */
			sscanf(token, "%c %d %d %d %d %d %u %llu %llu "
			       "%llu %llu %llu %llu", &c_temp, &i_temp,
			       &i_temp, &i_temp, &i_temp, &i_temp,
			       (unsigned *)&i_temp, &temp, &temp, &temp,
			       &temp, &utime, &stime);
		}
	}
	fclose(f);
	return utime + stime;
}

/* MAIN FUNCTIONS */

/**
 * @brief Initialize process list structure.
 *
 * Resets count to zero and clears internal CPU history buffers.
 *
 * @param plist Pointer to process list to initialize.
 */
void proc_list_init(proc_list_t *plist) {
	plist->count = 0;
	/* Clear history on startup */
	memset(cpu_history, 0, sizeof(cpu_history));
}

/**
 * @brief Update process list by reading /proc directory.
 *
 * Scans /proc for running processes, calculates CPU usage since last update,
 * and populates list with current data. CPU calculation uses delta method
 * comparing process ticks against system ticks between updates.
 *
 * @param plist Pointer to process list to update.
 */
void proc_list_update(proc_list_t *plist) {
	DIR *dir;
	struct dirent *entry;

	/* Calculate system time NOW */
	unsigned long long current_system_time = get_system_time();
	unsigned long long system_delta = 0;

	if (prev_system_time > 0) {
		system_delta = current_system_time - prev_system_time;
	}

	/* Get number of cores to scale percentage (0-100% * cores) */
	long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cores < 1) {
		num_cores = 1;
	}

	dir = opendir("/proc");
	if (!dir) {
		return;
	}

	plist->count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (plist->count >= MAX_PROCESSES) {
			break;
		}

		/* Processes are directories with numeric names */
		if (isdigit(entry->d_name[0])) {
			int pid = atoi(entry->d_name);
			proc_info_t *proc = &plist->list[plist->count];

			/* Fill basic info */
			proc->pid = pid;
			read_process_name(pid, proc->name,
					  sizeof(proc->name));
			proc->memory = read_process_memory(pid);
			read_process_user(pid, proc->user,
					  sizeof(proc->user));

			/* CPU CALCULATION */
			unsigned long long current_proc_time =
				get_process_time(pid);
			unsigned long long proc_delta = 0;

			/* Check bounds for history array */
			if (pid < 131072) {
				/*
				 * If we have history for this PID
				 * and time is valid
				 */
				if (cpu_history[pid] > 0 &&
				    current_proc_time >= cpu_history[pid]) {
					proc_delta = current_proc_time -
						     cpu_history[pid];
				}
				/* Save current time for next update frame */
				cpu_history[pid] = current_proc_time;
			}

			/* Calculate percentage */
			if (system_delta > 0) {
				/*
				 * Formula: (Process Delta / System Delta)
				 * * 100 * Cores
				 */
				proc->cpu_usage = (float)proc_delta /
						  (float)system_delta *
						  100.0 * num_cores;
			} else {
				proc->cpu_usage = 0.0;
			}

			plist->count++;
		}
	}
	closedir(dir);

	/* Save system time for next update */
	prev_system_time = current_system_time;
}

/**
 * @brief Filter process list based on search string.
 *
 * Performs case-insensitive search. If filter string matches process name,
 * that process is copied to destination list.
 *
 * @param src Pointer to source list (all processes).
 * @param dest Pointer to destination list (filtered processes).
 * @param filter_str String to search for. If NULL or empty, copies all.
 */
void proc_list_filter(const proc_list_t *src, proc_list_t *dest,
		      const char *filter_str) {
	if (!filter_str || strlen(filter_str) == 0) {
		*dest = *src;
		return;
	}

	dest->count = 0;
	for (int i = 0; i < src->count; i++) {
		/*
		 * Use strcasestr (non-standard GNU extension,
		 * enabled by _GNU_SOURCE)
		 */
		if (strcasestr(src->list[i].name, filter_str)) {
			dest->list[dest->count] = src->list[i];
			dest->count++;
		}
	}
}

/**
 * @brief Send termination signal to process.
 *
 * Uses SIGTERM for graceful shutdown. Could use SIGKILL for forced kill.
 *
 * @param pid Process ID to kill.
 * @return 0 on success, -1 on failure (errno is set).
 */
int proc_kill_process(pid_t pid) {
	/*
	 * Send SIGTERM (15) for graceful shutdown.
	 * Could use SIGKILL (9) for forced termination.
	 */
	return kill(pid, SIGTERM);
}