CC          = gcc
HPDEMO_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ -msse4 \
                     -I. -I$(CUDA_DIR)/include -I/usr/local/include \
                     -L. -L/usr/local/lib \
                     -lhashpipe -lrt -lm
HPDEMO_LIB_TARGET   = demo1_hashpipe.so
HPDEMO_LIB_SOURCES  = demo1_net_thread.c \
		      demo1_gpu_thread.c \
		      demo1_output_thread.c \
                      demo1_databuf.c
HPDEMO_LIB_INCLUDES = demo1_databuf.h

all: $(HPDEMO_LIB_TARGET)

$(HPDEMO_LIB_TARGET): $(HPDEMO_LIB_SOURCES) $(HPDEMO_LIB_INCLUDES)
	$(CC) -o $(HPDEMO_LIB_TARGET) $(HPDEMO_LIB_SOURCES) $(HPDEMO_LIB_CCFLAGS)

tags:
	ctags -R .
clean:
	rm -f $(HPDEMO_LIB_TARGET) tags

read: data-read
	./data-read

data-read: data-read.c
	$(CC) -o data-read data-read.c


prefix=/usr/local
LIBDIR=$(prefix)/lib
BINDIR=$(prefix)/bin
install-lib: $(HPDEMO_LIB_TARGET)
	mkdir -p "$(DESTDIR)$(LIBDIR)"
	install -p $^ "$(DESTDIR)$(LIBDIR)"
install: install-lib

.PHONY: all tags clean install install-lib
# vi: set ts=8 noet :
