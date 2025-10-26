run:
	@gcc src/main.c -o bin/main.exe && ./bin/main.exe

run_errors:
	@gcc src/errors/enums.c -o bin/errors.exe && ./bin/errors.exe