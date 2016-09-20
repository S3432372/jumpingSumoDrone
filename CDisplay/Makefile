# your Makefile

 #### variables
 RM= rm -vf
 CXX= gcc
 CXXFLAGS= -Wall -g
 CPPFLAGS= `pkg-config --cflags opencv`
 LDLIBS= `pkg-config --libs opencv`
 SOURCEFILES= DisplayImage.c
 OBJECTFILES= $(patsubst %.c,%.o,$(SOURCEFILES))
 PROGNAME= DisplayImage

 ### rules
 .PHONY: all clean

 all: $(PROGNAME)
 $(PROGNAME): $(OBJECTFILES)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

 clean:
	$(RM) $(OBJECTFILES) $(PROGNAME) 