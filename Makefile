PROJECTS = libcompy compy
PROFILE = debug




include .make/base




install: libcompy
	@echo $(SECTION_EOL)
	@echo -n "> cp bin/libcompy.so /usr/lib/libcompy.so"
	@cp bin/libcompy.so /usr/lib/libcompy.so
	@echo " (done)"

test: compy
	@echo $(SECTION_EOL)
	@echo Running tests...
	@test/all.sh
	@echo Done!
