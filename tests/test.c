/**
 * @file test.c
 * @brief Unit tests for the Process Browser core logic.
 *
 * Uses the Criterion framework to test sorting, filtering, and
 * data collection functions. UI functions are excluded from these tests.
 */

#include <criterion/criterion.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/proc.h"
#include "../src/sort.h"

/**
 * @brief Setup fixture.
 * Runs before each test in the suite (if attached).
 */
void setup(void) {
    // Optional: Reset global state or allocate resources
}

/**
 * @brief Teardown fixture.
 * Runs after each test in the suite.
 */
void teardown(void) {
    // Optional: Free resources
}

// --- Sort Suite ---

/**
 * @brief Test: Sort processes by PID (Ascending).
 */
Test(sort_suite, sort_by_pid, .init = setup, .fini = teardown) {
    proc_list_t plist;
    plist.count = 3;
    plist.list[0].pid = 100;
    plist.list[1].pid = 10;
    plist.list[2].pid = 50;

    sort_processes(&plist, SORT_PID);

    cr_assert_eq(plist.list[0].pid, 10, "First PID should be 10");
    cr_assert_eq(plist.list[1].pid, 50, "Second PID should be 50");
    cr_assert_eq(plist.list[2].pid, 100, "Third PID should be 100");
}

/**
 * @brief Test: Sort processes by Memory (Descending).
 */
Test(sort_suite, sort_by_mem_desc) {
    proc_list_t plist;
    plist.count = 3;
    plist.list[0].memory = 1024;
    plist.list[1].memory = 4096;
    plist.list[2].memory = 2048;

    sort_processes(&plist, SORT_MEM);

    // Memory is sorted largest to smallest
    cr_assert_eq(plist.list[0].memory, 4096, "Largest memory should be first");
    cr_assert_eq(plist.list[1].memory, 2048);
    cr_assert_eq(plist.list[2].memory, 1024);
}

// --- Filter Suite ---

/**
 * @brief Test: Filter finds a matching substring.
 */
Test(filter_suite, filter_match) {
    proc_list_t src, dest;
    src.count = 2;
    snprintf(src.list[0].name, 20, "systemd");
    snprintf(src.list[1].name, 20, "bash");

    // Search for "sys"
    proc_list_filter(&src, &dest, "sys");

    cr_assert_eq(dest.count, 1, "Should find exactly one match");
    cr_assert_str_eq(dest.list[0].name, "systemd", "Matched name should be systemd");
}

/**
 * @brief Test: Filter returns nothing when no match is found.
 */
Test(filter_suite, filter_no_match) {
    proc_list_t src, dest;
    src.count = 1;
    snprintf(src.list[0].name, 20, "bash");

    // Search for non-existent string
    proc_list_filter(&src, &dest, "xyz");

    cr_assert_eq(dest.count, 0, "Count should be 0 for no match");
}

/**
 * @brief Test: Empty filter string returns all processes.
 */
Test(filter_suite, filter_empty) {
    proc_list_t src, dest;
    src.count = 1;
    snprintf(src.list[0].name, 20, "bash");

    // Empty filter -> copy everything
    proc_list_filter(&src, &dest, "");

    cr_assert_eq(dest.count, 1, "Empty filter should return all processes");
}

// --- Logic Suite ---

/**
 * @brief Test: proc_list_update actually fetches data from system.
 *
 * This is an integration test validating that the code can read /proc
 * without crashing and find at least the init process.
 */
Test(proc_suite, proc_update) {
    proc_list_t plist;
    proc_list_init(&plist);
    proc_list_update(&plist);

    cr_assert_gt(plist.count, 0, "Should find at least one process on a Linux system");
    cr_assert_gt(plist.list[0].pid, 0, "PID should be positive");
}