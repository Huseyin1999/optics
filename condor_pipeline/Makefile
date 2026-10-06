CXX ?= g++
CXXFLAGS ?= -O2 -std=c++11 -Wall -fexceptions
CPPFLAGS += -I../TAIGA_optics/include -I../TAIGA_optics
LDLIBS += -ldl -lstdc++fs -lgsl -lgslcblas

CORE_NAMES = Photon Common Deviations LightAdaptor Material Mirror Optics Parameters \
             Atmosphere TriangulationScalar Triangulation_D SignalOut Additional
CORE_OBJECTS = $(addprefix build/src/,$(addsuffix .o,$(CORE_NAMES)))
OBJECTS = $(CORE_OBJECTS) build/direct_main.o
TARGET = TAIGA_optics_file

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDLIBS)

build/direct_main.o: direct_main.cpp ../TAIGA_optics/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/src/%.o: ../TAIGA_optics/src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)
