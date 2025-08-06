# This file is part of JCWM.
# 
# JCWM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#
# JCWM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with JCWM. If not, see <https://www.gnu.org/licenses/>.
#
#
TARGET = jcwm
TARGET2 = jcbar
TARGET3 = jcat
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
	sudo cp $(TARGET3) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET3)
clean:
	rm -f $(TARGET)
	rm -f $(TARGET2)
	rm -f $(TARGET3)
.PHONY: all install clean
