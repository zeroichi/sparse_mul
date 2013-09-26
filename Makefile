
CC=gcc
CPP=g++
NVCC=nvcc
TARGETS=cpu cpug gpu gpug vec_gen crs_gen
#CFLAGS=-g
CFLAGS=-O2
CPPFLAGS=$(CFLAGS)
NVCCFLAGS=$(CFLAGS)

SIZE?=1048576
NONZEROS?=1000000

CRS_FILE=data/matrix_$(SIZE)_$(NONZEROS).crs.bin
VEC_FILE=data/vector_$(SIZE).vec

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o

cpu: hoge.o crs.o vec.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

cpug: hogeg.o crs.o vec.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

hogeg.o: hoge.c
	$(CC) $(CFLAGS) -DWITH_GATHER -o $@ -c $<

gpu: hoge_gpu.cu crs.o vec.o misc.o
	$(NVCC) -o $@ $^

gpug: hoge_gpu.cu crs.o vec.o misc.o
	$(NVCC) -DWITH_GATHER -o $@ $^

vec_gen: vec_gen.o vec.o
	$(CC) $(CFLAGS) -o $@ $^

crs_gen: crs_gen.o crs.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

.cu.o:
	$(NVCC) $(NVCCFLAGS) -c $<

$(CRS_FILE): crs_gen
	./crs_gen -r $(SIZE) -c $(SIZE) -n $(NONZEROS) -o $@

$(VEC_FILE): vec_gen
	./vec_gen $(SIZE) > $@

test: test1 test2 test3 test4
	md5sum output_$(SIZE)_$(NONZEROS)_*.txt

test1: cpu $(CRS_FILE) $(VEC_FILE)
	./$< $(CRS_FILE) $(VEC_FILE) 2> output_$(SIZE)_$(NONZEROS)_$<.txt
	@echo

test2: cpug $(CRS_FILE) $(VEC_FILE)
	./$< $(CRS_FILE) $(VEC_FILE) 2> output_$(SIZE)_$(NONZEROS)_$<.txt
	@echo

test3: gpu $(CRS_FILE) $(VEC_FILE)
	./$< $(CRS_FILE) $(VEC_FILE) 2> output_$(SIZE)_$(NONZEROS)_$<.txt
	@echo

test4: gpug $(CRS_FILE) $(VEC_FILE)
	./$< $(CRS_FILE) $(VEC_FILE) 2> output_$(SIZE)_$(NONZEROS)_$<.txt
	@echo

.PHONY: all clean test test1 test2 test3 test4
.SUFFIXES: .c .cu .o
