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

2. Process Management
    - **Description:** Allows users to manage running processes by terminating unresponsive or problematic ones directly from the application interface.
    - **Acceptance criteria:**
      - Users can select a process from the list and terminate it with a single action.
      - The application requests confirmation before terminating a process.
      - Terminated processes are removed from the list or clearly marked as stopped.
      - Permission-related or system errors are handled gracefully and communicated to the user.

3. Process Sorting and Filtering
    - **Description:** Provides sorting and filtering capabilities to help users analyze system performance more effectively.
    - **Acceptance criteria:**
      - Users can sort processes by CPU usage, memory usage, process name, or PID.
      - Users can filter or search for processes by name.
      - Sorting and filtering are applied instantly without restarting the application.

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
