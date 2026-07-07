ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe 
CXXFLAGS     += $(ROOTCFLAGS)

OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4)
OPENCV_LIBS   := $(shell pkg-config --libs opencv4)


LD           := $(CXX)
LDFLAGS      := -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS      += -g -shared  #?? -Wl -rpath

EXTHEADERS    = -I../util
#UTIL         := ../util

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))

OSUFFIX := .o

# -- Source code
FILES = compound.o modulesAnalysis.o
MODULE_BINARIES = runModulesAnalysis parseRobotLog
OPENCV_BINARIES = ocvMarkersFromJPGV0 ocvMarkersFromJPGV1 ocvMarkersFromJPG
BINARIES = $(MODULE_BINARIES) $(OPENCV_BINARIES) $(ROBOT_BINARIES)
BIN_OBJS = $(addprefix obj/,$(addsuffix .o,$(BINARIES)))

LIBRARIES = lib/libAnaUtil.so

# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) $(OPENCV_CFLAGS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) $(OPENCV_CFLAGS) -c $< -o $@

# -- Targets
all: prep lib bin

lib: prep $(addprefix obj/,$(FILES))
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libModuleMeasurements.so $(LIBRARIES) $(GLIBS)

bin: lib $(addprefix bin/,$(BINARIES))

bin/%: obj/%.o lib/libModuleMeasurements.so
	$(LD) $(LDFLAGS) -o $@ $< lib/libModuleMeasurements.so $(LIBRARIES) $(GLIBS) \
		$(if $(filter $(notdir $@),$(OPENCV_BINARIES)),$(OPENCV_LIBS))


# -- create directories if not yet existing
prep:
	mkdir -p obj bin lib
	cd lib && rm -f libAnaUtil.so && ln -s /Users/ursl/macros/ana/util/lib/libAnaUtil.so  && cd -

clean:
	rm -f $(addprefix obj/,$(FILES)) $(BIN_OBJS)
	rm -rf bin/* obj/*
	rm -f lib/libModuleMeasurements.so
	rm -f lib/*.pcm

