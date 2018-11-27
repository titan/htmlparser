TARGET = libhtmlparser.a htmlparser.so

CC := gcc
CFLAGS := -g -O3 -fPIC -Wextra -Wall
PYCFLAGS := `pkg-config --cflags python-3.7`
PYLDFLAGS := `pkg-config --libs python-3.7`
AR := ar

LIBOBJS := htmlparser.o html-fsm.o attr-fsm.o
PYOBJS := py_htmlparser.o
OBJS := $(LIBOBJS) $(PYOBJS)

all: $(TARGET)

libhtmlparser.a: $(LIBOBJS)
	$(AR) -rcs $@ $(LIBOBJS)

htmlparser.so: $(PYOBJS) $(LIBOBJS)
	$(CC) $(PYLDFLAGS) $(PYOBJS) $(LIBOBJS) -shared -o $@

clean:
	rm -rf *-fsm.c *-fsm.h
	rm -rf *.o *.d
	rm -rf $(TARGET)

html-fsm.c: html-fsm.txt
	fsm-generator.py $< -d . --prefix html

attr-fsm.c: attr-fsm.txt
	fsm-generator.py $< -d . --prefix attr

%.o: %.c
	$(CC) -c $(CFLAGS) $(PYCFLAGS) $< -o $@

%.d: %.c
	rm -rf $@;
	$(CC) -MM $(CFLAGS) $(PYCFLAGS) $< > $@;
	sed -i 's,/($*/)/.o[ :]*,/1.o $@ : ,g' $@;

sinclude $(OBJS:.o=.d)

.PHONY: all clean
