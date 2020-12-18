CC          := g++

TARGET      := redis-hive

SRCDIR      := src
TESTDIR     := test
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := cpp
OBJEXT      := o

CFLAGS      := -Wall -O3 -g -MMD
LIB         := 
INC         := -I$(INCDIR) -I/usr/local/include

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
TESTS       := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/src/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
TESTOBJECTS := $(patsubst $(TESTDIR)/%,$(BUILDDIR)/test/%,$(TESTS:.$(SRCEXT)=.$(OBJEXT)))
DEP         := $(OBJECTS:%.o=%.d)
TESTDEP     := $(TESTOBJECTS:%.o=%.d)

all: $(TARGETDIR) $(BUILDDIR) $(TARGET) $(TARGET)_test

-include $(DEP)
-include $(TESTDEP)

clean:
	rm -rf $(BUILDDIR) $(TARGETDIR) data

$(TARGETDIR):
	@mkdir -p $(TARGETDIR)
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)/src
	@mkdir -p $(BUILDDIR)/test

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $(OBJECTS) $(LIB)

$(TARGET)_test: $(TESTOBJECTS) $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET)_test $(TESTOBJECTS) $(filter-out obj/src/main.o,$(OBJECTS)) -Lcppunit/lib -lcppunit $(LIB)

test:
	./$(TARGETDIR)/$(TARGET)_test

integration:
	rm -rf data
	cd integration; bundle install && bundle exec rspec . -fd

$(BUILDDIR)/src/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/test/%.$(OBJEXT): $(TESTDIR)/%.$(SRCEXT) cppunit
	$(CC) $(CFLAGS) $(INC) -Isrc -Icppunit/include -c -o $@ $<

cppunit.tgz:
	curl --fail -H "PRIVATE-TOKEN: ${TOKEN}" https://gitlab.com/api/v4/projects/22845459/packages/generic/cppunit/1.14.0/cppunit-linux-x86_64.tgz -o cppunit.tgz

cppunit: cppunit.tgz
	tar zxf cppunit.tgz

.PHONY: all clean test integration
