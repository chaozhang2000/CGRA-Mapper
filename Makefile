TARGET:= ./build/src/libmapperPass.so
SRC_DIR :=./src
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
H_FILES := $(wildcard $(SRC_DIR)/*.h)
HPP_FILES := $(wildcard $(SRC_DIR)/*.hpp)
.PHONY: clean
$(TARGET) : $(CPP_FILES) $(H_FILES) $(HPP_FILES)
	@mkdir -p ./build
	@cd ./build && cmake .. && make -j 20

clean :
	rm -rf build

