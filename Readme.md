[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/w1PSME4Z)
# Process Browser

A system utility that provides a comprehensive view of running processes on a computer. The application displays real-time information about CPU usage, memory consumption, and other relevant metrics for each process, allowing users to monitor system performance, identify resource-intensive tasks, and terminate problematic ones.

## Features

1. Real-Time Process Monitoring
    - **Description:** Displays all running processes in real time with key metrics such as CPU usage, memory consumption, process ID (PID), process name, and user.
    - **Acceptance criteria:**
      - The process list updates automatically at a defined interval (e.g., every 1–2 seconds).
      - CPU and memory usage are accurately displayed for each process.
      - Users can easily identify processes with high resource consumption.

2. Text-Based User Interface (TUI)
    - **Description:** Provides a terminal-based interface for interacting with processes, including navigation, viewing details, and performing actions.
    - **Acceptance criteria:**
      - Users can navigate through the process list using keyboard shortcuts.
      - Users can select a process and perform actions such as viewing details or terminating it with confirmation.
      - The interface updates in real time without requiring manual refresh.

3. Process Sorting and Filtering
    - **Description:** Enables users to sort and filter processes to quickly find specific tasks or analyze resource usage patterns.
    - **Acceptance criteria:**
      - Users can sort processes by CPU usage, memory usage, process name, or PID.
      - Users can filter or search for processes by name.
      - Sorting and filtering can be applied dynamically within the TUI.

## Dependencies (example, change according to your project)

- GCC
- Make

## Build Instructions (example, change according to your project)

1. Clone the repository:
```bash
git clone 
cd project-name
```

2. Build the project:
```bash
make
```

3. Run tests:
```bash
make test
```

## Usage Examples (example, change according to your project)

```bash
./program_name [arguments]
```
