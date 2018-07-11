PROJECTS = libcompy compy
PROFILE = debug

include .make/base

install: libcompy
	@echo $(strip $(SECTION_EOL))
	@echo -n "> cp bin/libcompy.so /usr/lib/libcompy.so"
	@cp bin/libcompy.so /usr/lib/libcompy.so
	@echo " (done)"
