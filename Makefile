# --- Settings ---
CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP -D_GNU_SOURCE
LDFLAGS = -lncurses

# Coverage flags
COV_FLAGS = --coverage

# Test-only libraries
TEST_LIBS = -lcriterion -lm

TARGET = pb
TEST_TARGET = run_tests

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Source files
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

TEST_SRC = tests/test.c
TEST_OBJ = $(TEST_SRC:.c=.o)

# Exclude main.o from test build
OBJ_NO_MAIN = $(filter-out src/main.o, $(OBJ))

DEPS = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

# --- Targets ---

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): CFLAGS += $(COV_FLAGS)
$(TEST_TARGET): $(OBJ_NO_MAIN) $(TEST_OBJ)
	$(CC) $(CFLAGS) $(OBJ_NO_MAIN) $(TEST_OBJ) -o $@ $(LDFLAGS) $(TEST_LIBS)

-include $(DEPS)

clean:
	rm -f src/*.o src/*.d tests/*.o tests/*.d $(TARGET) $(TEST_TARGET)
	rm -f *.gcno *.gcda *.gcov src/*.gcno src/*.gcda tests/*.gcno tests/*.gcda
	rm -rf coverage_report coverage.info

# Main test target
test: install_deps_test $(TEST_TARGET)
	@echo "\n>>> 1. UNIT TESTS <<<"
	@rm -f *.gcda src/*.gcda tests/*.gcda
	@./$(TEST_TARGET) --short
	@echo "\n>>> 2. LEAK CHECK (Valgrind) <<<"
	@valgrind -q --leak-check=full --error-exitcode=1 --errors-for-leak-kinds=definite ./$(TEST_TARGET) --short
	@echo "No memory leaks."
	@echo "\n>>> 3. SMOKE TEST <<<"
	@timeout 1s ./$(TARGET) > /dev/null 2>&1; \
	RET=$$?; \
	if [ $$RET -eq 124 ]; then \
		echo "Smoke test PASSED"; \
	else \
		echo "Smoke test FAILED (Code: $$RET)"; exit 1; \
	fi
	@echo "\n>>> 4. COVERAGE REPORT <<<"
	@if [ -x "$$(command -v lcov)" ]; then \
		lcov --capture --directory . --output-file coverage.info --quiet 2>/dev/null; \
		lcov --remove coverage.info \
			'/usr/*' \
			'tests/*' \
			'src/ui.c' \
			'src/main.c' \
			--output-file coverage.info --quiet 2>/dev/null; \
		genhtml coverage.info --output-directory coverage_report --quiet 2>/dev/null; \
		echo "Report: coverage_report/index.html"; \
	else \
		echo "Install lcov to see HTML coverage report."; \
	fi

install_deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y libncurses5-dev libncursesw5-dev libcriterion-dev lcov build-essential valgrind

install_deps_test:
	@if ! dpkg -s libcriterion-dev >/dev/null 2>&1; then \
		echo "Error: 'libcriterion-dev' not found. Please run 'make install_deps' first."; \
		exit 1; \
	fi

install: $(TARGET)
	mkdir -p $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

.PHONY: all clean test install_deps install uninstall install_deps_test