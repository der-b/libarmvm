.PHONY: delete_build doc

.SILENT:
%: build/CMakeCache.txt
	make -C build --no-print-directory $@ 

.SILENT:
build/CMakeCache.txt: build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
	make -C build --no-print-directory

run: build/CMakeCache.txt
	valgrind --leak-check=yes ./build/arm-vm -s 1000 -p ./example_programs/arithmetic/main.bin

run_gdb: build/CMakeCache.txt
	gdb --ex=r --args  ./build/arm-vm -s 1000 -p ./example_programs/arithmetic/main.bin


libopencm3:
	TARGETS=stm32/f0 make -C example_programs/libopencm3

example_programs: libopencm3
	make -C example_programs/endless_loop1
	make -C example_programs/endless_loop2
	make -C example_programs/arithmetic

build:
	mkdir build 	

delete_build:
	rm -rf build


doc:
	doxygen doxygen.conf
