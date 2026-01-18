/**
 * @file main.c
 * @brief Entry point of Process Browser application
 *
 * Contains main loop orchestrating Model (proc.c), View (ui.c),
 * and Controller logic (input handling).
 */

#include "proc.h"
#include "ui.h"
#include "sort.h"
#include <ncurses.h>
#include <string.h>

/**
 * @brief Main application function
 *
 * Initializes process list and UI, then enters main event loop.
 * Handles user input, updates process data, filters, sorts, and renders UI.
 *
 * @return 0 on successful execution
 */
int main() {
	proc_list_t all_processes;
	proc_list_t visible_processes;

	int running = 1;
	int selected = 0;
	int scroll_offset = 0;
	SortType current_sort = SORT_PID;

	char filter[50] = {0};
	int search_mode = 0;

	/* Flag to trigger confirmation dialog overlay */
	int kill_confirm_mode = 0;

	/* Initialization */
	proc_list_init(&all_processes);
	ui_init();

	while (running) {
		/*
		 * Update Model (only if not searching or in dialog
		 * to prevent UI jitter)
		 */
		if (!search_mode && !kill_confirm_mode) {
			proc_list_update(&all_processes);
		}

		/* Filter -> Sort */
		proc_list_filter(&all_processes, &visible_processes, filter);
		sort_processes(&visible_processes, current_sort);

		/* Bounds checking for selection */
		if (visible_processes.count == 0) {
			selected = 0;
		} else if (selected >= visible_processes.count) {
			selected = visible_processes.count - 1;
		}

		/* Render View */
		ui_draw(&visible_processes, selected, scroll_offset, filter,
			search_mode);

		/* If in confirmation mode, draw overlay dialog */
		if (kill_confirm_mode && visible_processes.count > 0) {
			ui_show_confirm_dialog(
				visible_processes.list[selected].name,
				visible_processes.list[selected].pid);
		}

		/* Handle Input */
		int ch = ui_handle_input();

		/* Kill Confirmation Mode */
		if (kill_confirm_mode) {
			if (ch == 'y' || ch == 'Y') {
				/* User confirmed kill */
				if (visible_processes.count > 0) {
					proc_kill_process(
					    visible_processes.list[selected].pid);
				}
				kill_confirm_mode = 0;
				/* Reduce timeout to update list immediately */
				timeout(100);
			} else if (ch == 'n' || ch == 'N' || ch == 27) {
				/* ESC or N */
				kill_confirm_mode = 0;
			}
			/* Ignore other keys in this mode */
			continue;
		}

		/* Search Mode */
		if (search_mode) {
			/* ESC or Enter to exit search */
			if (ch == 27 || ch == '\n') {
				search_mode = 0;
				timeout(1000); /* Restore normal update rate */
			} else if (ch == KEY_BACKSPACE || ch == 127) {
				int len = strlen(filter);
				if (len > 0)
					filter[len - 1] = 0;
			} else if (ch >= 32 && ch <= 126 &&
				   strlen(filter) < 49) {
				int len = strlen(filter);
				filter[len] = (char)ch;
				filter[len + 1] = 0;
				selected = 0;
				scroll_offset = 0;
			}
			continue;
		}

		/* Normal Navigation */
		int max_y = getmaxy(stdscr);
		int list_height = max_y - 2;

		switch (ch) {
		case 'q':
			running = 0;
			break;

		case 'k':          /* Vim style kill */
		case KEY_F(9):     /* Htop style kill */
			if (visible_processes.count > 0)
				kill_confirm_mode = 1;
			break;

		case '/':
			search_mode = 1;
			timeout(-1); /* Disable timeout while typing */
			break;

		case 27: /* ESC clears filter */
			filter[0] = 0;
			break;

		/* Sorting shortcuts */
		case 'p':
			current_sort = SORT_PID;
			selected = 0;
			scroll_offset = 0;
			break;

		case 'n':
			current_sort = SORT_NAME;
			selected = 0;
			scroll_offset = 0;
			break;

		case 'm':
			current_sort = SORT_MEM;
			selected = 0;
			scroll_offset = 0;
			break;

		case 'c':
			current_sort = SORT_CPU;
			selected = 0;
			scroll_offset = 0;
			break;

		/* Navigation */
		case KEY_UP:
			if (selected > 0) {
				selected--;
				if (selected < scroll_offset)
					scroll_offset = selected;
			}
			break;

		case KEY_DOWN:
			if (selected < visible_processes.count - 1) {
				selected++;
				if (selected >= scroll_offset + list_height)
					scroll_offset++;
			}
			break;
		}
	}

	ui_close();
	return 0;
}