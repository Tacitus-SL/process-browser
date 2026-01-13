#ifndef UI_H
#define UI_H

#include "proc.h"

/**
 * @brief Initializes the TUI (Text User Interface).
 *
 * Sets up ncurses, disables line buffering, hides the cursor,
 * and initializes color pairs.
 */
void ui_init();

/**
 * @brief Closes the TUI and restores the terminal settings.
 *
 * Should be called before the program exits.
 */
void ui_close();

/**
 * @brief Renders the process list and interface elements to the screen.
 *
 * Draws the table header, the list of processes (handling scrolling),
 * and the footer with status information.
 *
 * @param plist Pointer to the list of processes to display (usually the filtered list).
 * @param selected_idx The index of the currently selected row in the list.
 * @param start_index The index of the first visible row (scroll offset).
 * @param filter_str Current filter string (to display in the footer).
 * @param search_mode Boolean flag: 1 if user is currently typing a search query, 0 otherwise.
 */
void ui_draw(const proc_list_t *plist, int selected_idx, int start_index, const char *filter_str, int search_mode);

/**
 * @brief Handles user keyboard input.
 *
 * @return The character code of the pressed key.
 */
int ui_handle_input();

/**
 * @brief Displays a confirmation dialog for killing a process.
 *
 * Draws a modal window over the main interface asking for confirmation.
 *
 * @param proc_name The name of the process selected for termination.
 * @param pid The PID of the process.
 */
void ui_show_confirm_dialog(const char *proc_name, int pid);

#endif // UI_H