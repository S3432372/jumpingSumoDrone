EXEC_NAME=SearchAndRescue

#ARDrone SDK Directory
SDK_DIR=/Users/Caltheduck/Desktop/sdk/out/Unix-base/staging/usr

IDIR=./
CC=gcc
CFLAGS=-I$(IDIR) -I $(SDK_DIR)/include \
`pkg-config --cflags opencv`

LDLIBS = `pkg-config --libs opencv`
OBJDIR=obj
LDIR = $(SDK_DIR)/lib

EXTERNAL_LIB=-lncurses

#LIBS=-L$(LDIR) -larcommands -larcontroller -lardiscovery \
-larmedia -larnetwork -larnetworkal -larstream -larsal- lardatatransfer -larutils -lcurl -lssl\
-larstream2 $(EXTERNAL_LIB)

#LIBS = \
$(LDIR)/libardatatransfer.a $(LDIR)/libardatatransfer.so\
$(LDIR)/libarcontroller.a $(LDIR)/libarcontroller.so\
$(LDIR)/libarstream2.a $(LDIR)/libarstream2.so\
$(LDIR)/libarsal.a $(LDIR)/libarsal.so\
$(LDIR)/libarcommands.a $(LDIR)/libarcommands.so\
$(LDIR)/libardiscovery.a $(LDIR)/libardiscovery.so\
$(LDIR)/libarmedia.a $(LDIR)/libarmedia.so\
$(LDIR)/libarnetwork.a $(LDIR)/libarnetwork.so\
$(LDIR)/libarnetworkal.a $(LDIR)/libarnetworkal.so\
$(LDIR)/libarstream.a $(LDIR)/libarstream.so\
$(LDIR)/libjson.a $(LDIR)/libjson.so\
$(LDIR)/libarutils.a $(LDIR)/libarutils.so\
$(LDIR)/libcurl.a $(LDIR)/libcurl.so\
$(EXTERNAL_LIB)

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

#LIBS_DBG=-L$(SDK_DIR)/lib -larsal_dbg -larcommands_dbg -larnetwork_dbg -larnetworkal_dbg -lardiscovery_dbg -larstream_dbg $(EXTERNAL_LIB)

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
