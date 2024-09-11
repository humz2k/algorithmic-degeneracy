SOURCE_DIR ?= src
BUILD_DIR ?= build

SOURCES := $(shell find $(SOURCE_DIR) -name '*.cpp') $(shell find $(SOURCE_DIR) -name '*.c')
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
OBJECTS := $(OBJECTS:%.c=$(BUILD_DIR)/%.o)

HEADERS := $(shell find $(SOURCE_DIR) -name '*.hpp')

INCLUDE_FLAGS ?= -I$(SOURCE_DIR) -I/opt/homebrew/Cellar/openssl@3/3.3.2/include -I/opt/homebrew/Cellar/boost/1.86.0/include

FLAGS ?= -pthread -L/opt/homebrew/Cellar/boost/1.86.0/lib -L/opt/homebrew/Cellar/openssl@3/3.3.2/lib -lssl -lcrypto -lboost_system

OPT ?= -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-inline

#CXX := clang++

main: driver
.PHONY: main

.SECONDARY: $(OBJECTS) $(DRIVER_OBJECTS)

driver: $(OBJECTS)
	mkdir -p $(@D)
	$(CXX) $(OPT) -std=c++17 -o $@ $^ $(INCLUDE_FLAGS) $(FLAGS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	mkdir -p $(@D)
	$(CXX) $(OPT) -std=c++17 -c -o $@ $< $(INCLUDE_FLAGS)

clean:
	rm -rf $(BUILD_DIR)
	rm driver
.PHONY: clean