.PHONY: all clean

all: watcher

clean:
	rm watcher

watcher: watcher.cxx
	g++ -o watcher watcher.cxx -Os -s -Wall -Wextra -Wl,-O1 \
	-ffunction-sections -fdata-sections -Wl,-gc-sections \
	-L /opt/fltk/lib -lfltk -I /opt/fltk/include \
	-Wl,-rpath,/opt/fltk/lib
