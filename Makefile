EXEC_NAME=JumpingSumoPiloting

#ARDrone SDK Directory
SDK_DIR=/<DIRECTORY_TO_SDK>/sdk/out/Unix-base/staging/usr

IDIR=./
CC=gcc
CFLAGS=-I$(IDIR) -I $(SDK_DIR)/include \
`pkg-config --cflags opencv`

LDLIBS = `pkg-config --libs opencv`
OBJDIR=obj
LDIR = $(SDK_DIR)/lib

EXTERNAL_LIB=-lncurses

LIBS = \
$(LDIR)/libardatatransfer.a\
$(LDIR)/libarcontroller.a\
$(LDIR)/libarstream2.a\
$(LDIR)/libarsal.a\
$(LDIR)/libarcommands.a\
$(LDIR)/libardiscovery.a\
$(LDIR)/libarmedia.a\
$(LDIR)/libarnetwork.a\
$(LDIR)/libarnetworkal.a\
$(LDIR)/libarstream.a\
$(LDIR)/libjson.a\
$(LDIR)/libarutils.a\
$(LDIR)/libcurl.a\
$(EXTERNAL_LIB)

#Dependencies
_DEPS = JumpingSumoPiloting.h ihm.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = JumpingSumoPiloting.o ihm.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

FIFO = video_fifo

all: $(EXEC_NAME)
	@[ -p $(FIFO) ] || mkfifo $(FIFO)

$(OBJDIR)/%.o: %.c $(DEPS)
	@ [ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@ $(CC) -c -o $@ $< $(CFLAGS)

$(EXEC_NAME): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(LDLIBS)

.PHONY: clean

clean:
	@ rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
	@ rm -rf $(OBJDIR)
	@ rm -r $(EXEC_NAME)

run:
	LD_LIBRARY_PATH=$(LDIR) ./$(EXEC_NAME)
