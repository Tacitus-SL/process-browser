[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/w1PSME4Z)
# Process Browser

A system utility that provides a comprehensive view of running processes on a computer. The application displays real-time information about CPU usage, memory consumption, and other relevant metrics for each process, allowing users to monitor system performance, identify resource-intensive tasks, and terminate problematic ones.

## Educational Project
Developed as part of Programming Fundamentals course at NUP 2025.

## Code Style

This project follows the **K&R (Kernighan and Ritchie)** coding style.

## Features

1. Real-Time Process Monitoring
    - **Description:** Displays all running processes in real time with key metrics such as CPU usage, memory consumption, process ID (PID), process name, and user.

2. Text-Based User Interface (TUI)
    - **Description:** Provides a terminal-based interface for interacting with processes, including navigation, viewing details, and performing actions.

3. Process Sorting and Filtering
    - **Description:** Enables users to sort and filter processes to quickly find specific tasks or analyze resource usage patterns.

## Documentation

All functions are documented using **Doxygen** docstring format.

## Dependencies
Build & Runtime:
- **gcc** - GNU C Compiler
- **make** - Build automation tool
- **libncurses5-dev** / **libncursesw5-dev** - TUI library

Testing:
- **libcriterion-dev** - Unit testing framework
- **valgrind** - Memory leak detection
- **lcov** - Code coverage HTML reports

## Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/programming-fundamentals-nup-2025/examproject2-Tacitus-SL
cd examproject2-Tacitus-SL.git
```

2. Install dependencies:
```bash
make install_deps
```

3. Build the project:
```bash
make
```

4. Run the process browser:
```bash
./pb
```

## Testing

Run all tests:
```bash
make test
```

This command executes:
- **Unit tests** (Criterion framework)
- **Valgrind leak check** (ensures zero memory leaks)
- **Smoke test** (verifies TUI launches without crashes)
- **Coverage report** (generates HTML report in `coverage_report/`)

View coverage report:
```bash
xdg-open coverage_report/index.html
```

## Usage

### Keyboard Controls

**Navigation:**
- `↑` / `↓` - Move selection up/down
- `q` - Quit application

**Sorting:**
- `p` - Sort by Process ID (PID)
- `n` - Sort by process Name (alphabetical)
- `m` - Sort by Memory usage (descending)
- `c` - Sort by CPU usage (descending)

**Actions:**
- `/` - Enter search/filter mode
- `ESC` - Clear filter (or exit search mode)
- `k` - Kill selected process (shows confirmation)
- `Y` / `N` - Confirm/Cancel kill operation


## Installation

Install to `/usr/local/bin`:
```bash
sudo make install
```

Uninstall:
```bash
sudo make uninstall
```

## Project Structure
```
.
├── src/
│   ├── main.c           # Entry point and main event loop
│   ├── proc.c/proc.h    # Process data collection from /proc
│   ├── sort.c/sort.h    # Sorting logic (PID, name, memory, CPU)
│   └── ui.c/ui.h        # TUI interface (ncurses)
├── tests/
│   └── test.c           # Criterion unit tests
├── Makefile             # Build system
├── README.md
└── .gitignore
```
