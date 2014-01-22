SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
TARGET=dlmalloc.a
CC=gcc
CFLAGS=-fPIC 
LDFLAGS=-lm 

all: .depend $(TARGET)

.depend: $(SOURCES)
	rm -f .depend
	$(CC) $(CFLAGS) -MM $^ >> .depend;

include .depend

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -Rf $(TARGET) $(OBJECTS) .depend