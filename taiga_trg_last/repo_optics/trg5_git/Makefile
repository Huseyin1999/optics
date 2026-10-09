CXX ?= g++
CXXFLAGS = -std=c++11 -O3 -Wall

all: cleaning trigger

# 1. Cleaning & Hillas parameters (pure C++11, no external dependencies)
cleaning: cleaning_corsika_pipeline.cpp
	$(CXX) $(CXXFLAGS) cleaning_corsika_pipeline.cpp -o cleaning

# 2. Trigger simulation (built-in Akima spline, only OpenMP, NO CERN ROOT or GSL dev headers required!)
trigger: readbin_v4.cpp
	$(CXX) $(CXXFLAGS) readbin_v4.cpp -o trigger_iact -fopenmp

# 3. Optional: Trigger simulation with CERN ROOT (if desired)
trigger_root: readbin_v4.cpp
	$(CXX) $(CXXFLAGS) -DUSE_ROOT readbin_v4.cpp -o trigger_iact -fopenmp `root-config --cflags --libs` -lMathMore

clean:
	rm -f cleaning trigger_iact
