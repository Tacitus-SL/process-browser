/**
 * @file ui.c
 * @brief Implementation of the Text User Interface using ncurses.
 */

#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Initialize the TUI (Text User Interface).
 *
 * Sets up ncurses, disables line buffering, hides cursor,
 * enables keyboard input, and initializes color pairs.
 */
void ui_init() {
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	timeout(1000);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_CYAN);   /* Header */
		init_pair(2, COLOR_WHITE, COLOR_RED);    /* Dialog */
		init_pair(3, COLOR_BLACK, COLOR_WHITE);  /* Selected row */
	}
}

/**
 * @brief Close the TUI and restore terminal settings.
 */
void ui_close() {
	endwin();
}

/**
 * @brief Render the process list and interface elements.
 *
 * Draws three sections: header (column names), process list (scrollable),
 * and footer (status/commands). Handles row highlighting for selection.
 *
 * @param plist Pointer to filtered/sorted process list to display.
 * @param selected_idx Index of currently selected process.
 * @param start_index First visible row index (scroll offset).
 * @param filter_str Current filter string (displayed in footer).
 * @param search_mode 1 if user is typing search query, 0 otherwise.
 */
void ui_draw(const proc_list_t *plist, int selected_idx, int start_index,
	     const char *filter_str, int search_mode) {
	clear();

	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);

	/* HEADER */
	attron(COLOR_PAIR(1) | A_BOLD);

	move(0, 0);
	/* Fill entire header row with colored spaces */
	for (int i = 0; i < max_x; i++) {
		addch(' ' | COLOR_PAIR(1) | A_BOLD);
	}

	/* Print column headers with proper alignment */
	mvprintw(0, 0, " %-6s %-20s %-12s %12s %8s",
		 "PID", "NAME", "USER", "MEM(kB)", "CPU%");

	attroff(COLOR_PAIR(1) | A_BOLD);

	int rows_available = max_y - 2;  /* Subtract header and footer */

	/* PROCESS LIST */
	for (int i = start_index;
	     i < plist->count && (i - start_index) < rows_available; i++) {
		int screen_line = (i - start_index) + 1;

		/* Truncate process name to 20 chars */
		char safe_name[21];
		strncpy(safe_name, plist->list[i].name, 20);
		safe_name[20] = '\0';

		/* Truncate username to 12 chars */
		char safe_user[13];
		strncpy(safe_user, plist->list[i].user, 12);
		safe_user[12] = '\0';

		/* Clamp memory display to avoid overflow */
		long mem_display = (plist->list[i].memory > 999999999999L) ?
				   999999999999L : plist->list[i].memory;

		/* Clamp CPU percentage to [0.0, 100.0] range */
		float cpu_display = plist->list[i].cpu_usage;
		if (cpu_display > 100.0f) {
			cpu_display = 100.0f;
		}
		if (cpu_display < 0.0f) {
			cpu_display = 0.0f;
		}

		/*
		 * Format data into large buffer to accommodate
		 * alignment spaces for full terminal width.
		 */
		char data_buffer[2048];
		int written = snprintf(data_buffer, sizeof(data_buffer),
				       " %-6d %-20s %-12s %12ld %8.1f",
				       plist->list[i].pid, safe_name,
				       safe_user, mem_display, cpu_display);

		/* Pad with spaces if line shorter than terminal width */
		if (written < max_x) {
			memset(data_buffer + written, ' ', max_x - written);
			data_buffer[max_x] = '\0';
		} else {
			data_buffer[max_x] = '\0';
		}

		/*
		 * Create chtype buffer for attribute application.
		 * This allows entire row to have background color.
		 */
		chtype *line_buffer = (chtype *)malloc(max_x *
						       sizeof(chtype));
		if (!line_buffer) {
			continue;
		}

		/* Selected row gets highlight, others normal */
		chtype attr = (i == selected_idx) ? COLOR_PAIR(3) : A_NORMAL;

		/* Apply attribute to each character */
		for (int j = 0; j < max_x; j++) {
			line_buffer[j] = (unsigned char)data_buffer[j] | attr;
		}

		/* Clear line before printing to avoid artifacts */
		move(screen_line, 0);
		clrtoeol();

		/* Print entire line at once */
		mvaddchnstr(screen_line, 0, line_buffer, max_x);

		free(line_buffer);
	}

	/* Clear any remaining lines below the list */
	for (int i = (plist->count - start_index); i < rows_available; i++) {
		move(i + 1, 0);
		clrtoeol();
	}

	/* FOOTER */
	move(max_y - 1, 0);
	clrtoeol();

	if (search_mode) {
		/* Show search prompt when filtering */
		attron(COLOR_PAIR(1) | A_BOLD);
		mvprintw(max_y - 1, 0, "SEARCH: %s_", filter_str);
		attroff(COLOR_PAIR(1) | A_BOLD);
	} else {
		/* Show help text and status */
		mvprintw(max_y - 1, 0,
			 "Sort: [p]id [n]ame [m]em [c]pu | [k]ill | "
			 "Filter: [%s] | Total: %d | [q]uit",
			 filter_str ? filter_str : "", plist->count);
	}

	refresh();
}

/**
 * @brief Display confirmation dialog for killing a process.
 *
 * Draws a centered modal window over the main interface,
 * prompting user to confirm or cancel the kill operation.
 *
 * @param proc_name Name of the process to be killed.
 * @param pid PID of the process to be killed.
 */
void ui_show_confirm_dialog(const char *proc_name, int pid)
{
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);

	int height = 5;
	int width = 60;
	int start_y = (max_y - height) / 2;
	int start_x = (max_x - width) / 2;

	if (start_x < 0)
		start_x = 0;
	if (start_y < 0)
		start_y = 0;

	if (has_colors())
		attron(COLOR_PAIR(2) | A_BOLD);

	/* Draw solid background block */
	for (int i = 0; i < height; i++) {
		move(start_y + i, start_x);
		for (int j = 0; j < width; j++)
			addch(' ');
	}

	/* Truncate name */
	char safe_name[40];
	snprintf(safe_name, sizeof(safe_name), "%s", proc_name);

	/* Content */
	mvprintw(start_y + 1, start_x + 2, "WARNING: Kill process?");
	mvprintw(start_y + 2, start_x + 2, "%s (PID: %d)", safe_name, pid);
	mvprintw(start_y + 3, start_x + 2,
		 "Press [Y] to Confirm  or  [N] to Cancel");

	if (has_colors())
		attroff(COLOR_PAIR(2) | A_BOLD);

	refresh();
}

/**
 * @brief Handle keyboard input from user.
 *
 * Blocks (or times out per timeout() setting) waiting for input.
 *
 * @return Character code of pressed key (including special keys like KEY_UP).
 */
int ui_handle_input() {
	return getch();
}