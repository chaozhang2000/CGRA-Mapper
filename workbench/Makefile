MAPPER_DIR := $(shell pwd)/..
MAPPER_SRC_DIR := $(MAPPER_DIR)/src
MAPPER_BUILD_DIR:= $(MAPPER_DIR)/build
MAPPER := $(MAPPER_BUILD_DIR)/src/libmapperPass.so
MAPPER_SRC_CPP := $(wildcard $(MAPPER_SRC_DIR)/*.cpp)
MAPPER_SRC_H := $(wildcard $(MAPPER_SRC_DIR)/*.h)
MAPPER_SRC_HPP := $(wildcard $(MAPPER_SRC_DIR)/*.hpp)

WORK_DIR:= $(shell pwd)
WORK_KERNEL_DIR:= $(WORK_DIR)/kernels
WORK_SCRIPT_DIR:= $(WORK_DIR)/scripts
WORK_BUILD_DIR:= $(WORK_DIR)/build
NAME ?= kernel
KERNEL_SRC := $(WORK_KERNEL_DIR)/$(NAME).cpp
KERNEL_LL := $(WORK_BUILD_DIR)/$(NAME)/$(NAME).ll
KERNEL_BC := $(WORK_BUILD_DIR)/$(NAME)/$(NAME).bc

map: $(MAPPER) $(KERNEL_BC)
	@bash $(WORK_SCRIPT_DIR)/run.sh $(MAPPER) $(KERNEL_BC) $(WORK_BUILD_DIR)/$(NAME)
$(MAPPER):$(MAPPER_SRC_CPP) $(MAPPER_SRC_H) $(MAPPER_SRC_HPP)
	@mkdir -p $(MAPPER_BUILD_DIR)
	@cd $(MAPPER_BUILD_DIR) && cmake .. && make -j 20
$(KERNEL_BC):$(KERNEL_SRC)
	@mkdir -p $(WORK_BUILD_DIR)/$(NAME)
	@cp $(KERNEL_SRC) $(WORK_BUILD_DIR)/$(NAME)
	@bash $(WORK_SCRIPT_DIR)/compile.sh $(KERNEL_SRC) $(KERNEL_BC) $(KERNEL_LL)
	@echo "\033[32m#compile $(NAME).cpp to $(NAME).ll and $(NAME).bc in $(WORK_BUILD_DIR)/$(NAME)\033 \033[0m"
kernel:$(KERNEL_BC)
mapper:$(MAPPER)
clean:
	rm -rf $(WORK_BUILD_DIR)/$(NAME)
cleanmapper:
	rm -rf $(MAPPER_BUILD_DIR)
cleanall:
	rm -rf $(MAPPER_BUILD_DIR) $(WORK_BUILD_DIR)
