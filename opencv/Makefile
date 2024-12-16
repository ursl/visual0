# ======================================================================
# NOTE: make all
# -----
#
# ======================================================================
.PHONY: prep clean vars

ifdef SCRAM_ARCH
  # CXX         := $(shell scramv1 tool info gcc-cxxcompiler | grep CXX= | sed s/CXX=//)
  # this likely is decrepit
  # CXX         := /cvmfs/cms.cern.ch/slc6_amd64_gcc530/external/gcc/5.3.0/bin/gcc
else
  ifdef CXXCOMPILER
    CXX         := $(CXXCOMPILER)
  else
    ifdef CXX
    CXX         := $(CXX)
    else 
      CXX         := c++
    endif
  endif
endif

INCOPENCV     := -I /opt/homebrew/Cellar/opencv/4.10.0_15/include/opencv4

CXXFLAGS      = -std=c++1z -g -O0 -Wall -fPIC -pipe
CXXFLAGS     += $(INCOPENCV)

LD            = $(CXX)
LDFLAGS       = $(CXXFLAGS)  -dynamiclib -shared
SOFLAGS       = $(CXXFLAGS)  -dynamiclib -shared

LIBOPENCV     := -L /opt/homebrew/Cellar/opencv/4.10.0_15/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

# -- Source code
OPENCV = opencvTests.o opencvModule.o

# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@


# -- Targets
all: prep lib bin

lib: $(addprefix obj/,$(OPENCV))
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(OPENCV)) -o lib/libOpenCV.so  $(LIBOPENCV)

bin: opencvRun.cc lib
	$(CXX) $(CXXFLAGS) -c -o obj/opencvRun.o opencvRun.cc 
	$(CXX) $(CXXFLAGS) -o bin/opencvRun opencvRun.cc lib/libOpenCV.so $(LIBOPENCV) 


# -- create directories if not yet existing
prep:
	mkdir -p obj bin lib

clean:
	rm -f obj/*.o
	rm -f bin/opencvRun
	rm -f lib/libOpenCV.so
