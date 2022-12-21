
LD_FLAGS_CAP = $(shell pkg-config --libs libcap)
TARGET = demo-cap

all: $(TARGET)

capget.o:capget.c
	gcc -c capget.c

capset.o:capset.c
	gcc -c capset.c

demo-cap.o:demo-cap.c
	gcc -c demo-cap.c

demo-cap: capget.o capset.o demo-cap.o
	gcc demo-cap.o capget.o capset.o -o demo-cap $(LD_FLAGS_CAP)


clean:
	rm -f *.o
	rm -f $(TARGET)

.PHONY: all clean