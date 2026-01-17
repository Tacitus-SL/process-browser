/**
 * @file sort.c
 * @brief Implementation of process sorting logic.
 */

#include "sort.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/**
 * @brief Comparator for PID sorting (ascending).
 *
 * @param a Pointer to first process.
 * @param b Pointer to second process.
 * @return Negative if a < b, positive if a > b, zero if equal.
 */
static int compare_pid(const void *a, const void *b) {
	const proc_info_t *pa = (const proc_info_t *)a;
	const proc_info_t *pb = (const proc_info_t *)b;
	return pa->pid - pb->pid;
}

/**
 * @brief Comparator for Memory sorting (descending).
 *
 * Uses safe comparison to avoid integer overflow with large memory values.
 *
 * @param a Pointer to first process.
 * @param b Pointer to second process.
 * @return Positive if b > a (larger first), negative if b < a, zero if equal.
 */
static int compare_mem(const void *a, const void *b) {
	const proc_info_t *pa = (const proc_info_t *)a;
	const proc_info_t *pb = (const proc_info_t *)b;

	/* Safe comparison to avoid overflow */
	if (pb->memory > pa->memory) {
		return 1;
	}
	if (pb->memory < pa->memory) {
		return -1;
	}
	return 0;
}

/**
 * @brief Comparator for Name sorting (alphabetical, case-insensitive).
 *
 * @param a Pointer to first process.
 * @param b Pointer to second process.
 * @return Result of strcasecmp (negative, zero, or positive).
 */
static int compare_name(const void *a, const void *b) {
	const proc_info_t *pa = (const proc_info_t *)a;
	const proc_info_t *pb = (const proc_info_t *)b;
	return strcasecmp(pa->name, pb->name);
}

/**
 * @brief Comparator for CPU usage sorting (descending).
 *
 * @param a Pointer to first process.
 * @param b Pointer to second process.
 * @return Positive if b > a (larger first), negative if b < a, zero if equal.
 */
static int compare_cpu(const void *a, const void *b) {
	const proc_info_t *pa = (const proc_info_t *)a;
	const proc_info_t *pb = (const proc_info_t *)b;

	if (pb->cpu_usage > pa->cpu_usage) {
		return 1;
	}
	if (pb->cpu_usage < pa->cpu_usage) {
		return -1;
	}
	return 0;
}

/**
 * @brief Sort the process list in place based on given criteria.
 *
 * @param plist Pointer to process list to sort.
 * @param type Sorting criteria (PID, NAME, MEM, or CPU).
 */
void sort_processes(proc_list_t *plist, SortType type) {
	switch (type) {
	case SORT_PID:
		qsort(plist->list, plist->count, sizeof(proc_info_t),
		      compare_pid);
		break;
	case SORT_NAME:
		qsort(plist->list, plist->count, sizeof(proc_info_t),
		      compare_name);
		break;
	case SORT_MEM:
		qsort(plist->list, plist->count, sizeof(proc_info_t),
		      compare_mem);
		break;
	case SORT_CPU:
		qsort(plist->list, plist->count, sizeof(proc_info_t),
		      compare_cpu);
		break;
	default:
		break;
	}
}