#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void ui_init() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_CYAN);
        init_pair(2, COLOR_WHITE, COLOR_RED);
        init_pair(3, COLOR_BLACK, COLOR_WHITE);
    }
}

void ui_close() {
    endwin();
}

void ui_draw(const proc_list_t *plist, int selected_idx, int start_index, const char *filter_str, int search_mode) {
    clear();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // --- 1. ЗАГОЛОВОК ---
    attron(COLOR_PAIR(1) | A_BOLD);

    move(0, 0);
    for (int i = 0; i < max_x; i++) {
        addch(' ' | COLOR_PAIR(1) | A_BOLD);
    }

    // Заголовок с правильным выравниванием
    mvprintw(0, 0, " %-6s %-20s %-12s %12s %8s",
        "PID", "NAME", "USER", "MEM(kB)", "CPU%"
    );

    attroff(COLOR_PAIR(1) | A_BOLD);

    int rows_available = max_y - 2;

    // --- 2. СПИСОК ---
    for (int i = start_index; i < plist->count && (i - start_index) < rows_available; i++) {
        int screen_line = (i - start_index) + 1;

        char safe_name[21];
        strncpy(safe_name, plist->list[i].name, 20);
        safe_name[20] = '\0';

        char safe_user[13];
        strncpy(safe_user, plist->list[i].user, 12);
        safe_user[12] = '\0';

        // Ограничиваем значения
        long mem_display = (plist->list[i].memory > 999999999999L) ? 999999999999L : plist->list[i].memory;
        // ОГРАНИЧИВАЕМ CPU до 100.0%
        float cpu_display = plist->list[i].cpu_usage;
        if (cpu_display > 100.0f) cpu_display = 100.0f;
        if (cpu_display < 0.0f) cpu_display = 0.0f;

        // Формируем данные в БОЛЬШОЙ буфер, чтобы поместились все пробелы выравнивания
        char data_buffer[2048];
        int written = snprintf(data_buffer, sizeof(data_buffer),
            " %-6d %-20s %-12s %12ld %8.1f",
            plist->list[i].pid,
            safe_name,
            safe_user,
            mem_display,
            cpu_display
        );

        // Если строка короче max_x, добиваем пробелами
        if (written < max_x) {
            memset(data_buffer + written, ' ', max_x - written);
            data_buffer[max_x] = '\0';
        } else {
            data_buffer[max_x] = '\0';
        }

        // Создаём буфер chtype
        chtype *line_buffer = (chtype *)malloc(max_x * sizeof(chtype));
        if (!line_buffer) continue;

        chtype attr = (i == selected_idx) ? COLOR_PAIR(3) : A_NORMAL;

        // Заполняем буфер - копируем РОВНО max_x символов
        for (int j = 0; j < max_x; j++) {
            line_buffer[j] = (unsigned char)data_buffer[j] | attr;
        }

        // Очищаем строку ПЕРЕД печатью
        move(screen_line, 0);
        clrtoeol();

        // Печатаем
        mvaddchnstr(screen_line, 0, line_buffer, max_x);

        free(line_buffer);
    }

    // Очищаем оставшиеся строки
    for (int i = (plist->count - start_index); i < rows_available; i++) {
        move(i + 1, 0);
        clrtoeol();
    }

    // --- 3. ФУТЕР ---
    move(max_y - 1, 0);
    clrtoeol();

    if (search_mode) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(max_y - 1, 0, "SEARCH: %s_", filter_str);
        attroff(COLOR_PAIR(1) | A_BOLD);
    } else {
        mvprintw(max_y - 1, 0,
            "Sort: [p]id [n]ame [m]em [c]pu | [k]ill | Filter: [%s] | Total: %d | [q]uit",
            filter_str ? filter_str : "", plist->count
        );
    }

    refresh();
}

void ui_show_confirm_dialog(const char *proc_name, int pid) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int height = 5;
    int width = 50;
    int start_y = (max_y - height) / 2;
    int start_x = (max_x - width) / 2;

    if (has_colors()) {
        attron(COLOR_PAIR(2) | A_BOLD);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            mvaddch(start_y + i, start_x + j, ' ');
        }
    }

    mvprintw(start_y + 1, start_x + 2, "WARNING: Kill process?");
    mvprintw(start_y + 2, start_x + 2, "%s (PID: %d)", proc_name, pid);
    mvprintw(start_y + 3, start_x + 2, "Press [Y] to Confirm or [N] to Cancel");

    if (has_colors()) {
        attroff(COLOR_PAIR(2) | A_BOLD);
    }

    refresh();
}

int ui_handle_input() {
    return getch();
}