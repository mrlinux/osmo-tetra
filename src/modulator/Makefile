TARGET=tetramod

DEBUG=n
CXX_SOURCES=\
	main.cpp \
	modulator.cpp \
	rootraisedcosinefilter.cpp \
	nco.cpp

CXXFLAGS=-std=gnu++0x -I. `pkg-config --cflags uhd`
LIBS=-lm `pkg-config --libs uhd`

DEPDIR=./deps

##############################################################################

CXX=g++
LD=g++

CXXOBJS=$(CXX_SOURCES:%.cpp=%.o)
DEPS=$(CXX_SOURCES:%.cpp=%.dep)
FULLDEPS=$(addprefix $(DEPDIR)/,$(DEPS))

ifeq ($(DEBUG),y)
	CXXFLAGS+=-O0 -g3 -Wall
	LDFLAGS+=-g3
else
	CXXFLAGS+=-O2 -Wall
	LDFLAGS+=-s
endif

all: build

build: $(DEPDIR) $(TARGET)

-include $(FULLDEPS)

$(TARGET): $(CXXOBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(CXXOBJS) $(LIBS)

$(CXXOBJS):
	$(CXX) $(CXXFLAGS) -MD -MP -MF $(patsubst %.o,$(DEPDIR)/%.dep, $@) -c $(patsubst %.o,%.cpp, $@) -o $@

$(DEPDIR):
	@mkdir -p $(DEPDIR)

clean:
	rm -Rf $(CXXOBJS) $(DEPDIR) $(TARGET) *~

distclean: clean

run: build
	./$(TARGET)

debug: build
	gdb ./$(TARGET)
