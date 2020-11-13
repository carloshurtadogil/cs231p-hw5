out:=hw5
.PHONY: all clean

all:
	gcc -Wall -pthread -o $(out).bin main.c util.c transpose.c -lm -g

clean:
	rm -rf $(out).bin
