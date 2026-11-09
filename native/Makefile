SDK_DIR = vendor/hollyhock-3/sdk
CROSS ?= sh4a_nofpueb-elf-
CXX_TARGET = $(CROSS)g++
CXX_HOST ?= g++
CORE = src/store.cpp src/canvas.cpp src/app.cpp src/screen_backup.cpp src/calculator.cpp
HEADERS = $(wildcard src/*.hpp) src/equations.inc
WARN = -Wall -Wextra -Wpedantic -Werror
FLAGS = -std=c++20 -Os -g -flto -ffat-lto-objects -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti $(WARN) -Isrc -I$(SDK_DIR)/include -DPHYSIK_HH3
OBJECTS = $(patsubst src/%.cpp,build/target/%.o,$(CORE) src/startup.cpp src/main.cpp)

.PHONY: hh3 host-test
hh3: dist/physik.hh3
host-test: build/host-tests build/sdk-file-tests build/calculator-tests

$(SDK_DIR)/libsdk.a:
	$(MAKE) -C $(SDK_DIR)

build/target/%.o: src/%.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	$(CXX_TARGET) $(FLAGS) -MMD -MP -c $< -o $@

dist/physik.elf: $(OBJECTS) $(SDK_DIR)/libsdk.a
	@mkdir -p $(@D)
	$(CXX_TARGET) $(FLAGS) -Wl,-Ttext-segment,0x8C052800,--gc-sections,-Map,dist/physik.map -o $@ $(OBJECTS) -L$(SDK_DIR) -lsdk -lm

dist/physik.hh3: dist/physik.elf
	$(CROSS)strip -o $@ $<
	$(CROSS)size $@

build/host-tests: $(CORE) tests/test.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) -std=c++20 -O1 -g $(WARN) -fsanitize=address,undefined -fno-omit-frame-pointer -Isrc $(CORE) tests/test.cpp -o $@

build/sdk-file-tests: src/store.cpp tests/test_sdk.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) -std=c++20 -O1 -g $(WARN) -fsanitize=address,undefined -fno-omit-frame-pointer -DPHYSIK_HH3 -Isrc -I$(SDK_DIR)/include src/store.cpp tests/test_sdk.cpp -o $@

build/calculator-tests: src/store.cpp src/calculator.cpp tests/test_calculator.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) -std=c++20 -O1 -g $(WARN) -fsanitize=address,undefined -fno-omit-frame-pointer -Isrc src/store.cpp src/calculator.cpp tests/test_calculator.cpp -o $@

-include $(OBJECTS:.o=.d)
