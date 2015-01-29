
all:
	@echo "Please do 'make thread' or 'make coroutine'"

thread:main.c coroutine.c coroutine.h
	gcc -D THREAD=1 -g -o co.elf $^ -lpthread

coroutine:main.c coroutine.c coroutine.h
	gcc -D THREAD=0 -g -o co.elf $^ -lpthread

clean:
	rm co.elf

test:co.elf
	time ./co.elf

debug:
	gdb ./co.elf

