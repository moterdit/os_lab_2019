CC = gcc
CFLAGS = -Wall
LIBS = -lm

all: /projects/os-lab-2019/lab3/src/parallel_min_max /projects/os-lab-2019/lab4/src/process_memory parallel_sum

/projects/os-lab-2019/lab3/src/parallel_min_max: /projects/os-lab-2019/lab3/src/parallel_min_max.c /projects/os-lab-2019/lab3/src/find_min_max.c /projects/os-lab-2019/lab3/src/utils.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

/projects/os-lab-2019/lab4/src/process_memory: /projects/os-lab-2019/lab4/src/process_memory.c
	$(CC) $(CFLAGS) -o $@ $^

parallel_sum: parallel_sum.c find_min_max.c utils.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -pthread

clean:
	rm -f /projects/os-lab-2019/lab3/src/parallel_min_max /projects/os-lab-2019/lab4/src/process_memory parallel_sum
