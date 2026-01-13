#ifndef SORT_H
#define SORT_H

#include "proc.h"

/**
 * @brief Enumeration defining available sorting criteria.
 */
typedef enum {
	SORT_PID,   /**< Sort by Process ID (Ascending) */
	SORT_NAME,  /**< Sort by Process Name (Alphabetical) */
	SORT_MEM,   /**< Sort by Memory usage (Descending) */
	SORT_CPU    /**< Sort by CPU usage (Descending) */
} SortType;

/**
 * @brief Sorts the process list in place.
 *
 * Uses standard qsort to order the process list based on the selected criteria.
 *
 * @param plist Pointer to the process list to sort.
 * @param type The sorting criteria (PID, NAME, MEM, or CPU).
 */
void sort_processes(proc_list_t *plist, SortType type);

#endif // SORT_H