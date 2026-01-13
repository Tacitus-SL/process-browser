/**
 * @file sort.c
 * @brief Implementation of process sorting logic.
 */

#include "sort.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h> // for strcasecmp

/**
 * @brief Comparator for PID sorting (Ascending).
 */
static int compare_pid(const void *a, const void *b) {
    const proc_info_t *pa = (const proc_info_t *)a;
    const proc_info_t *pb = (const proc_info_t *)b;
    return pa->pid - pb->pid;
}

/**
 * @brief Comparator for Memory sorting (Descending).
 */
static int compare_mem(const void *a, const void *b) {
    const proc_info_t *pa = (const proc_info_t *)a;
    const proc_info_t *pb = (const proc_info_t *)b;
    // Return (b - a) so larger values come first
    return (int)(pb->memory - pa->memory);
}

/**
 * @brief Comparator for Name sorting (Alphabetical / Case-insensitive).
 */
static int compare_name(const void *a, const void *b) {
    const proc_info_t *pa = (const proc_info_t *)a;
    const proc_info_t *pb = (const proc_info_t *)b;
    return strcasecmp(pa->name, pb->name);
}

/**
 * @brief Comparator for CPU usage sorting (Descending).
 */
static int compare_cpu(const void *a, const void *b) {
    const proc_info_t *pa = (const proc_info_t *)a;
    const proc_info_t *pb = (const proc_info_t *)b;

    if (pb->cpu_usage > pa->cpu_usage) return 1;
    if (pb->cpu_usage < pa->cpu_usage) return -1;
    return 0;
}

void sort_processes(proc_list_t *plist, SortType type) {
    switch (type) {
    case SORT_PID:
        qsort(plist->list, plist->count, sizeof(proc_info_t), compare_pid);
        break;
    case SORT_NAME:
        qsort(plist->list, plist->count, sizeof(proc_info_t), compare_name);
        break;
    case SORT_MEM:
        qsort(plist->list, plist->count, sizeof(proc_info_t), compare_mem);
        break;
    case SORT_CPU:
        qsort(plist->list, plist->count, sizeof(proc_info_t), compare_cpu);
        break;
    default:
        break;
    }
}