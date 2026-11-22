SDK_DIR = vendor/hollyhock-3/sdk
CROSS ?= sh4a_nofpueb-elf-
CXX_TARGET = $(CROSS)g++
CXX_HOST ?= g++
CORE = src/store.cpp src/canvas.cpp src/app.cpp src/screen_backup.cpp src/calculator.cpp src/kinematics.cpp src/extensions.cpp src/catalog_data.cpp src/plot_view.cpp src/knowledge.cpp src/navigation.cpp src/mechanics.cpp src/fields.cpp src/optics.cpp src/analysis.cpp src/simulation.cpp
WARN = -Wall -Wextra -Wpedantic -Werror
FLAGS = -std=c++20 -Os -g -flto -ffat-lto-objects -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics $(WARN) -Isrc -I$(SDK_DIR)/include -DPHYSIK_HH3 -DPHYSIK_DATA_CATALOG
HOST_FLAGS = -std=c++20 -O1 -g $(WARN) -fsanitize=address,undefined -fno-omit-frame-pointer -Isrc
OBJECTS = $(patsubst src/%.cpp,build/target/%.o,$(CORE) src/startup.cpp src/main.cpp)
HOST_OBJECTS = $(patsubst src/%.cpp,build/host/%.o,$(CORE))
COMPACT_CORE = src/extensions.cpp src/catalog_data.cpp src/mechanics.cpp src/fields.cpp src/optics.cpp src/analysis.cpp src/simulation.cpp
COMPACT_OBJECTS = $(patsubst src/%.cpp,build/compact/%.o,$(COMPACT_CORE)) $(filter-out $(patsubst src/%.cpp,build/host/%.o,$(COMPACT_CORE)),$(HOST_OBJECTS))
EXPORT_OBJECTS = $(filter-out build/host/app.o build/host/canvas.o build/host/screen_backup.o build/host/plot_view.o build/host/knowledge.o build/host/navigation.o,$(HOST_OBJECTS))

.PHONY: hh3 host-test compact-test
hh3: dist/physik.hh3
host-test: build/host-tests build/sdk-file-tests build/calculator-tests build/kinematics-tests build/mechanics-tests build/fields-tests build/optics-tests build/analysis-tests build/simulation-tests build/extensions-tests build/navigation-tests
compact-test: build/compact-host-tests build/compact-mechanics-tests build/compact-fields-tests build/compact-optics-tests build/compact-analysis-tests build/compact-simulation-tests build/compact-extensions-tests build/compact-catalog-tests build/compact-navigation-tests

$(SDK_DIR)/libsdk.a:
	$(MAKE) -C $(SDK_DIR)

build/target/%.o: src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_TARGET) $(FLAGS) -MMD -MP -c $< -o $@

dist/physik.elf: $(OBJECTS) $(SDK_DIR)/libsdk.a
	@mkdir -p $(@D)
	$(CXX_TARGET) $(FLAGS) -Wl,-Ttext-segment,0x8C052800,--gc-sections,-Map,dist/physik.map -o $@ $(OBJECTS) -L$(SDK_DIR) -lsdk -lm

dist/physik.hh3: dist/physik.elf
	$(CROSS)strip -o $@ $<
	$(CROSS)size $@

build/host/%.o: src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -MMD -MP -c $< -o $@

build/compact/%.o: src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -DPHYSIK_DATA_CATALOG -MMD -MP -c $< -o $@

build/test/%.o: tests/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -MMD -MP -c $< -o $@

build/compact-test/test_catalog.o: tests/test_catalog.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -DPHYSIK_DATA_CATALOG -MMD -MP -c $< -o $@

build/test/export_catalog.o: tools/export_catalog.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -MMD -MP -c $< -o $@

build/export-catalog: $(EXPORT_OBJECTS) build/test/export_catalog.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/host-tests: $(HOST_OBJECTS) build/test/test.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/%-tests: $(HOST_OBJECTS) build/test/test_%.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/compact-host-tests: $(COMPACT_OBJECTS) build/test/test.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/compact-catalog-tests: $(COMPACT_OBJECTS) build/compact-test/test_catalog.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/compact-%-tests: $(COMPACT_OBJECTS) build/test/test_%.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/sdk-file-tests: src/store.cpp src/store.hpp src/paths.hpp tests/test_sdk.cpp Makefile
	@mkdir -p $(@D)
	$(CXX_HOST) $(HOST_FLAGS) -DPHYSIK_HH3 -I$(SDK_DIR)/include src/store.cpp tests/test_sdk.cpp -o $@

build/calculator-tests: build/host/store.o build/host/calculator.o build/test/test_calculator.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

build/kinematics-tests: build/host/store.o build/host/calculator.o build/host/kinematics.o build/test/test_kinematics.o
	$(CXX_HOST) $(HOST_FLAGS) $^ -o $@

.SECONDARY:
-include $(OBJECTS:.o=.d) $(HOST_OBJECTS:.o=.d) $(patsubst src/%.cpp,build/compact/%.d,$(COMPACT_CORE)) $(wildcard build/test/*.d build/compact-test/*.d)
