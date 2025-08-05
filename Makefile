
TARGET = jcwm
TARGET2 = jcbar
SRC = main.c
SRC2 = jcbar.c
CC = gcc
CFLAGS = -g -O0
LDFLAGS = -lX11 -lXrandr
INCLUDES = -I/usr/include/X11/extensions/

all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS) $(INCLUDES)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2) $(LDFLAGS) $(INCLUDES)
install: $(TARGET)
	sudo mv $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo mv $(TARGET2) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET2)
clean:
	rm -f $(TARGET)
	rm -f $(TARGET2)
.PHONY: all install clean
