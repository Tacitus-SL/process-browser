# Instructions
## General
1. Implement your project in the repository. 
2. Put all required information in the readme file (you are provided with a template)
3. You should have rich test coverage:
   1. For projects that build executables:
      1. For all functions (which are not responsible for UI) you should have a suite of unit tests with at least 80% coverage.
      2. You should have end-to-end tests for your entire program with every possible IO scenario (all combinations of flags and parameters). If your program creates GUI or TUI – not required.
      3. If your program creates GUI or TUI – you should have a smoke test
   2. For projects that build libraries:
      1. For all functions you should have a suite of unit tests with at least 90% coverage.
   3. Unit tests must be written using any testing framework you want (but not with bare `assert`)
## Requirements
1. Your Makefile should:
   1. Rebuild only changed files
   2. Use templates and automatic dependencies resolver
   3. Have 6 mandatory targets: 
      1. all – builds the project
      2. clean – removes all build artifacts
      3. test – runs all tests for the project
      4. install_deps – installs all dependencies (do nothing if no dependencies are specified)
      5. install – installs the project 
      6. uninstall – uninstalls the project
2. You must use one of C code styles: K&R, Allman (BSD), GNU, Linux Kernel or Google (specify in the readme which one you used)
3. Your program or library (if used correctly) MUST NOT produce any memory leaks.
4. Your project MUST be well-documented (with Doxygen docstrings, for example, but you can use any other doc string style)
5. All source and header files should be in `src` directory (you can create any tree structure in it)
6. You must list all dependencies in the readme file (in **the Dependencies** section)


## Marking criteria:
1. 20 points for each implemented feature (max 60)
2. 5 points for code style and documentation
3. 5 points for project structure (no .o and binary files, multiple files, etc.)
4. 15 points for the correct Makefile
5. 15 points for tests
