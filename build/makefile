#edit following config
TOP=..
BIN = psim
CC = g++
SYMBOLS = _ALLBUILD _EXCLUDECPEU _INCLUDE_PROFILER _PERFBUILD
OPTIMIZ = -O2
DEBUG_LEVEL = -g3

#do not edit
OTHER_FLAGS = -Wall -m32 -c -fmessage-length=0
HEADERS = $(shell find $(TOP) -name '*.h')
SRCS = $(shell find $(TOP) -name '*.cpp')
OBJS = $(patsubst ../%, %, $(SRCS:.cpp=.o))
HEADER_DIRS = $(sort $(dir $(HEADERS)))
SRC_DIRS = $(sort $(dir $(SRCS)))
BUILD_DIR = $(shell pwd)
INCLUDE=$(addprefix -I,$(HEADER_DIRS))
DEFSYM=$(addprefix -D,$(SYMBOLS))
CFLAGS = $(INCLUDE) $(OTHER_FLAGS) $(DEFSYM) $(OPTIMIZ) $(DEBUG_LEVEL)
vpath %.cpp $(addsuffix :,$(SRC_DIRS))
OBJS_DIR = $(patsubst ../%, %, $(SRC_DIRS))

all: $(BUILD_DIR) make_subdir $(BIN)

$(BUILD_DIR):
	-if ! [ -d $@ ] ; then mkdir $@ ; fi

make_subdir:
	@for i in $(OBJS_DIR) ; do \
		if ! [ -d $$i ] ; then mkdir -p $$i ; fi \
	done

$(BIN): $(OBJS)
	$(CC) $^ -o $(BIN)

$(OBJS) : %.o : $(TOP)/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	@echo $(CC) $(CFLAGS) -o $@ $<
	@$(CC) $(CFLAGS) -o $@ $< && \
	echo -n $(@:%.o=%.d) $(dir $@) > $(@:%.o=%.d) && \
	$(CC) -MM -MG -P -w $(CFLAGS) $< >> $(@:%.o=%.d)
	@echo 'Finished building: $<'
	@echo ' '

.PHONY: clean make_subdir

clean:
	-rm -Rf $(OBJS_DIR) $(OBJS) $(BIN) common

