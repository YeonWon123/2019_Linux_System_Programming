CC = gcc
OBJECTS = ssu_backup.o
TARGET = ssu_backup

.SUFFIXES : .c .o

all : $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) -lpthread

clean:
	rm -f $(OBJECTS) $(TARGETS)
