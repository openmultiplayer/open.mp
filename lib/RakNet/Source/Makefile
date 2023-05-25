include $(BASE_DIR)/makefile.defs

all: shared static

shared:
	mkdir -p $(BASE_DIR)/Lib/linux/
	$(CC) $(DEBUG) -I$(INCLUDE) -w -c *.cpp
	$(CC) $(DEBUG) -shared -Wl-soname,libraknet.so.2 -o $(BASE_DIR)/Lib/linux/libraknet.so.$(VERSION) *.o $(LIBS)

static:
	mkdir -p $(BASE_DIR)/Lib/linux/
	$(CC) $(DEBUG) -I$(INCLUDE) -w -c *.cpp
	$(AR) rcs $(BASE_DIR)/Lib/linux/libraknet.a *.o

clean:
	rm -f core
	rm -f *.o
	rm -f $(BASE_DIR)/Lib/linux/libraknet.so.$(VERSION)
	rm -f $(BASE_DIR)/Lib/linux/linuxraknet.a
