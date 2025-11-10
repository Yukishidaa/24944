all: t10 test_binaries

t10: task_10.c
	gcc -o t10 task_10.c

test_binaries: test_exit0 test_exit5 test_exit57 test_signal

test_exit0: test_exit0.c
	gcc -o test_exit0 test_exit0.c

test_exit5: test_exit5.c
	gcc -o test_exit5 test_exit5.c

test_exit57: test_exit57.c
	gcc -o test_exit57 test_exit57.c

test_signal: test_signal.c
	gcc -o test_signal test_signal.c

test: all
	@echo "=== Тест 1: Нормальная программа ==="
	./t10 ls -la
	@echo "=== Тест 2: Код возврата 0 ==="
	./t10 ./test_exit0
	@echo "=== Тест 3: Код возврата 5 ==="
	./t10 ./test_exit5
	@echo "=== Тест 4: Код возврата 57 ==="
	./t10 ./test_exit57
	@echo "=== Тест 5: Завершение по сигналу ==="
	./t10 ./test_signal
	@echo "=== Тест 6: Ошибка - нет аргументов ==="
	./t10 || true
	@echo "=== Тест 7: Ошибка - несуществующая программа ==="
	./t10 nonexistent_program || true

clean:
	rm -f t10 test_exit0 test_exit5 test_exit57 test_signal

.PHONY: all test clean test_binaries