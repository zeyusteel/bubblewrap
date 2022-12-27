
LD_FLAGS_CAP = $(shell pkg-config --libs libcap)
TARGET = demo-cap demo-prctl demo-test demo-eventfd \
			demo-sigchild demo-clone demo-newroot

all: $(TARGET)

capget.o:capget.c
	gcc -c capget.c

capset.o:capset.c
	gcc -c capset.c

demo-cap.o:demo-cap.c
	gcc -c demo-cap.c

demo-cap: capget.o capset.o demo-cap.o
	gcc demo-cap.o capget.o capset.o -o demo-cap $(LD_FLAGS_CAP)

demo-prctl.o: demo-prctl.c
	gcc -c demo-prctl.c

demo-prctl: demo-prctl.o capget.o capset.o
	gcc demo-prctl.o capget.o capset.o -o demo-prctl $(LD_FLAGS_CAP)

demo-test.o: demo-test.c
	gcc -c demo-test.c

demo-test: demo-test.o
	gcc demo-test.o -o demo-test

eventfd.o: eventfd.c
	gcc -c eventfd.c

demo-eventfd: eventfd.o
	gcc eventfd.o -o demo-eventfd

sigchild.o: sigchild.c
	gcc -c sigchild.c

demo-sigchild: sigchild.o
	gcc sigchild.o -o demo-sigchild

raw-clone.o: raw-clone.c
	gcc -c raw-clone.c

demo-clone: raw-clone.o capget.o
	gcc raw-clone.o capget.o -o demo-clone $(LD_FLAGS_CAP)

pivot-root.o: pivot-root.c
	gcc -c pivot-root.c

demo-newroot: pivot-root.o
	gcc pivot-root.o -o demo-newroot

clean:
	rm -f *.o
	rm -f $(TARGET)

.PHONY: all clean