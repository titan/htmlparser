TARGET = libhtmlparser.a htmlparser.so

CC := gcc
CFLAGS := -g -O3 -fPIC -Wextra -Wall
PYCFLAGS := `pkg-config --cflags python-3.7`
PYLDFLAGS := `pkg-config --libs python-3.7`
AR := ar

LIBOBJS := html-fsm.o html-fsm-actions.o attr-fsm.o attr-fsm-actions.o htmlparser.o
PYOBJS := py_htmlparser.o

all: $(TARGET)

libhtmlparser.a: $(LIBOBJS)
	$(AR) rcs $@ $(LIBOBJS)
	ranlib $@

htmlparser.so: $(LIBOBJS) $(PYOBJS)
	$(CC) $(PYLDFLAGS) $(PYOBJS) $(LIBOBJS) -shared -o $@

clean:
	rm -rf *-fsm.c *-fsm.h
	rm -rf *.o *.d
	rm -rf $(TARGET)

html-fsm.c: html-fsm.txt
	naive-fsm-generator.py $< -d . --prefix html

attr-fsm.c: attr-fsm.txt
	naive-fsm-generator.py $< -d . --prefix attr

%.o: %.c
	$(CC) -c $(CFLAGS) $(PYCFLAGS) $< -o $@

%.d: %.c
	rm -rf $@;
	$(CC) -MM $(CFLAGS) $(PYCFLAGS) $< > $@;
	sed -i 's,/($*/)/.o[ :]*,/1.o $@ : ,g' $@;

sinclude $(OBJS:.o=.d)

.PHONY: all clean
