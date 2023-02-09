# umk is Ultimate++ make CLI tool to build U++ projects
# use https://www.ultimatepp.org/ for further instructions how to install U++ for your platform
# build rule expects "aoc-2022" Assembly to exist (in U++ package selection) and CLANG20 build method

# on my box
# - upp/.config/u++/ide/aoc-2022.var
### UPP = "/home/ped/coding/adventofcode/2022-upp;/home/ped/upp/uppsrc";
### OUTPUT = "/home/ped/upp/.cache/upp.out";
### UPPHUB = "";
# - upp/.config/u++/ide/CLANG20.bm
# is identical to default CLANG.bm, only common C++ options are modified to enable C++20:
### COMMON_CPP_OPTIONS = "-std=c++20 -Wno-logical-op-parentheses";

### configured as

.PHONY: all, time

BUILD_DIR := build
UPP_FILES := $(wildcard */*.upp)
UPP_FILES_BASE := $(notdir $(UPP_FILES) )
BIN_FILES := $(addprefix $(BUILD_DIR)/, $(patsubst %.upp,%, $(basename $(UPP_FILES_BASE) ) ) )

$(BUILD_DIR)/% : */%.upp
	mkdir -p $(@D)
	umk aoc-2022 $(<D) -sbr CLANG20 $@

all: $(BIN_FILES)

time: $(BIN_FILES)
	./time_all.bash
	diff expected.log $(BUILD_DIR)/__out__.log
