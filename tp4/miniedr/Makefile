CLANG ?= clang
CC ?= gcc
BPFTOOL ?= bpftool

ARCH := $(shell uname -m | sed 's/x86_64/x86/')

BPF_CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH) \
              -I/usr/include -I.

CFLAGS := -g -O2 -Wall -I.

all: vmlinux.h miniedr jailer

vmlinux.h:
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

miniedr.bpf.o: miniedr.bpf.c vmlinux.h miniedr.h
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

miniedr.skel.h: miniedr.bpf.o
	$(BPFTOOL) gen skeleton $< > $@

miniedr: miniedr.c miniedr.skel.h miniedr.h
	$(CC) $(CFLAGS) -o $@ $< -lbpf -lelf -lz

jailer: jailer.c miniedr.h
	$(CC) $(CFLAGS) -o $@ $< -lbpf

clean:
	rm -f vmlinux.h *.o *.skel.h miniedr jailer

.PHONY: all clean
