include build-config.mk

.PHONY: all clean

all: showpath
clean:
	$(RM) *.o showpath
