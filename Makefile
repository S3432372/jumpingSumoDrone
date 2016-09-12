# your Makefile

 #### variables
 RM= rm -vf
 CXX= gcc
 CXXFLAGS= -Wall -g
 CPPFLAGS= `pkg-config --cflags opencv`
 LDLIBS= `pkg-config --libs opencv`
 SOURCESTREAM= openCVStream.c
 OBJECTSTREAM= $(patsubst %.c,%.o,$(SOURCESTREAM))
 PROGSTREAM= openCVStream

 SOURCEBALL= ball_detection.c
 OBJECTBALL= $(patsubst %.c,%.o,$(SOURCEBALL))
 PROGBALL= ball_detection

 ### rules
 .PHONY: all clean stream color

 all: stream color

 stream: $(PROGSTREAM)
 $(PROGSTREAM): $(OBJECTSTREAM)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

 color: $(PROGBALL)
 $(PROGBALL): $(OBJECTBALL)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

 clean:
	$(RM) $(OBJECTSTREAM) $(OBJECTBALL) $(PROGSTREAM) $(PROGBALL) 