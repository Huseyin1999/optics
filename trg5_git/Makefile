all: main

main:
	#g++ cleaning_corsika_auto.cpp -o cleaning -Wall -Wfloat-equal -Warray-bounds -Wdiv-by-zero -Wextra -Wconversion
	g++ -std=c++11 cleaning_corsika_pipeline.cpp -o cleaning -Wall -Wfloat-equal -Warray-bounds -Wdiv-by-zero -Wextra -Wconversion
	#g++ find_corsica_stereo_auto.cxx -o find_stereo -Wall -Wfloat-equal -Warray-bounds -Wdiv-by-zero -Wextra -Wconversion
	#g++ trigger_iact234.cpp -o trigger_iact234 -lMathMore -fopenmp -I `root-config --incdir` `root-config --cflags` `root-config --libs`
	#g++ trigger_iact1.cpp -o trigger_iact1 -lMathMore -fopenmp -I `root-config --incdir` `root-config --cflags` `root-config --libs`
	#g++ -std=c++11 readbin_v4.cpp -o trigger_iact -lMathMore -fopenmp -I `root-config --incdir` `root-config --cflags` `root-config --libs`
	#g++ readbin_v3_4_1_iact2.cpp -o trigger_iact2 -lMathMore -fopenmp -I `root-config --incdir` `root-config --cflags` `root-config --libs`
clean:
	rm cleaning
