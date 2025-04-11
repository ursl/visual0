ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe 
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS      := -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS      += -g -shared  #?? -Wl -rpath

EXTHEADERS    = -I../util
#UTIL         := ../util

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))

OSUFFIX := .o

# -- Source code
FILES = compound.o moduleMeasurement.o 
BINARIES = runModuleMeasurement

LIBRARIES = lib/libAnaUtil.so 


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS)  $(EXTHEADERS) -c $< -o $@
 
$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS)   $(EXTHEADERS) -c $< -o $@

# -- Targets
all: prep lib bin

# -- targets
all: prep lib bin
 
lib: $(addprefix obj/,$(FILES))
	echo "hallo lib"
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libModuleMeasurements.so  $(LIBRARIES) $(GLIBS)
 
bin: lib obj/runModuleMeasurement.o
	echo "hallo bin"
	$(LD) $(LDFLAGS) -o bin/runModuleMeasurement obj/runModuleMeasurement.o lib/libModuleMeasurements.so  $(LIBRARIES) $(GLIBS) 


# -- create directories if not yet existing
prep:
	mkdir -p obj bin lib
	cd lib && rm -f libAnaUtil.so && ln -s /Users/ursl/macros/ana/util/lib/libAnaUtil.so  && cd -

clean:
	rm -f $(addprefix obj/,$(FILES) $(DICT))
	rm -rf bin/* obj/*
	rm -f lib/libModuleMeasurements.so
	rm -f lib/*.pcm

