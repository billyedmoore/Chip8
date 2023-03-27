build:
		cc src/main.c src/peripheral.c src/cpu.c -lSDL2
clean:
		rm -rf a.out
