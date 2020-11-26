CC          := g++

TARGET      := redis-hive

SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := cpp
OBJEXT      := o

CFLAGS      := -Wall -O3 -g -MMD
LIB         := 
INC         := -I$(INCDIR) -I/usr/local/include

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
DEP         := $(OBJECTS:%.o=%.d)

all: $(TARGETDIR) $(BUILDDIR) $(TARGET)

-include $(DEP)

clean:
	rm -rf $(BUILDDIR) $(TARGETDIR)

$(TARGETDIR):
	@mkdir -p $(TARGETDIR)
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $(OBJECTS) $(LIB)

$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

.PHONY: all clean
