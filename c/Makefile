# Some references:
# https://docs.oracle.com/cd/E19504-01/802-5880/6i9k05dhg/index.html
BUILD_DIR = out
LOG_DIR = logs
OUTPUT_DIRS = ${BUILD_DIR} ${LOG_DIR}

CC = /usr/local/opt/llvm/bin/clang #this compiler supports OpenMP
CFLAGS = -I/usr/local/include -g -Wall -Wno-missing-braces -Wextra -Wshadow -Wpedantic -std=c11 -fopenmp #comment out openmp flag to run sequential version of nlogn_solution
LDFLAGS = 
SRCS_PROD := quadratic_sol.c nlogn_sol.c multiproc_sol.c multithread_sol.c util.c
OBJS_PROD := $(addprefix $(BUILD_DIR)/, $(patsubst %.c,%.o,$(SRCS_PROD)))
LDLIBS =


#if __linux__
	VALGRIND = valgrind --tool=memcheck --leak-check=full 
	VALGRIND += --verbose --log-file=${LOG_DIR}/valgrind.log
#else
	VALGRIND = 
#endif


.PHONY: all clean unittest filetest benchmarktest

all: clean unittest filetest benchmarktest


$(OBJS_PROD): | ${OUTPUT_DIRS}

####################### 
#### test goals  ######
#######################

unittest: $(BUILD_DIR)/closest_point_test
	$(VALGRIND) ./$^

$(BUILD_DIR)/closest_point_test: $(OBJS_PROD) ${BUILD_DIR}/closest_point_test.o
	$(LINK.c) $^ -o $@ $(LDLIBS) -L/usr/local/opt/cmocka/lib -lcmocka

filetest: $(BUILD_DIR)/file_test
	$(VALGRIND) ./$^ $(filename)

$(BUILD_DIR)/file_test: $(OBJS_PROD) ${BUILD_DIR}/file_test.o
	$(LINK.c) $^ -o $@ $(LDLIBS)

benchmarktest: $(BUILD_DIR)/benchmark_test
	$(VALGRIND) ./$^ $(testmode) $(param)

$(BUILD_DIR)/benchmark_test: $(OBJS_PROD) ${BUILD_DIR}/benchmark_test.o
	$(LINK.c) $^ -o $@ $(LDLIBS)

testfile: $(BUILD_DIR)/test_file
	$(VALGRIND) ./$^ $(filename) $(numpoints)

$(BUILD_DIR)/test_file: ${BUILD_DIR}/test_file.o ${BUILD_DIR}/util.o
	$(LINK.c) $^ -o $@ $(LDLIBS)


# if an object ﬁle is needed, compile the corresponding .c ﬁle
${BUILD_DIR}/%.o: %.c
	$(COMPILE.c) $< -o $@


${OUTPUT_DIRS}:
	mkdir $@

clean:
	${RM} -r ${LOG_DIR}/* ${BUILD_DIR}/* *.o