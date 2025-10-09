CC := clang
CXX := clang++
MAKE ?= make

EXE := physMP
SRC_DIR := src
SRC_DIRS := $(shell find ${SRC_DIR}/ -type d)
# TODO: figure out how to make this one command
SRC := $(shell find ${SRC_DIR}/ -type f -name '*.c')
SRC += $(shell find ${SRC_DIR}/ -type f -name '*.m')
SRC_CXX := $(shell find ${SRC_DIR}/ -type f -name '*.cpp')
RES := resources
# TODO: make sure the base name has at least one character
RES_SRC := $(shell find ${RES}/ -name '*.*')

SHDR_SRC := ${SRC_DIR}/metal/shaders
SHDR_METAL := $(wildcard ${SHDR_SRC}/*.metal)

OBJ_DIR = ${OUT_DIR}/objects
OBJ_C = $(patsubst src/%.c,${OBJ_DIR}/%.o,${SRC})
OBJ = $(patsubst src/%.m,${OBJ_DIR}/%.o,${OBJ_C})
OBJ_CXX = $(patsubst src/%.cpp,${OBJ_DIR}/%.cxx.o,${SRC_CXX})
OBJ_DIRS = $(patsubst ${SRC_DIR}/%,${OBJ_DIR}/%,${SRC_DIRS})
RES_DIR = ${OUT_DIR}/resources
SHDR_DIR = ${RES_DIR}/shaders
RES_OUT = $(patsubst ${RES}/%,${RES_DIR}/%,${RES_SRC})

SHDR_AIR_OUT = $(patsubst ${SHDR_SRC}/%.metal,${OBJ_DIR}/%.air,${SHDR_METAL})

SCRIPT_SRC := scripts/jolt.make
JOLT_DIR := jolt/Build
OUT_JOLT_DIR := ${JOLT_DIR}/Linux_Debug
OUT_JOLT := ${OUT_JOLT_DIR}/libJolt.a

override LIB += Jolt m pthread sdl3
override FRAMEWORK += Foundation Metal
override LIB_PATH += /usr/local/lib jolt/Build/Linux_Debug
override INCL_PATH += src jolt /usr/local/include

LIB_FL := $(patsubst %,-l%,${LIB})
FRAMEWORK_FL := $(patsubst %, -framework %, ${FRAMEWORK})
LIB_PATH_FL := $(patsubst %, -L%, ${LIB_PATH})
INCL_PATH_FL := $(patsubst %, -I%, ${INCL_PATH})

.PHONY: all clean fullclean

OUT_DIR := build
OUT := ${OUT_DIR}/${EXE}

O ?= 2

override CCFLAGS += -flto -funsafe-math-optimizations -fno-math-errno -fvisibility=hidden

all: ${OBJ_DIRS} ${OUT} ${SHDR_DIR}/default.metallib ${RES_OUT}

${OUT}: ${OBJ} ${OBJ_CXX}
	${CXX} $^ -O$O -o $@ ${LIB_PATH_FL} ${LIB_FL} ${FRAMEWORK_FL} ${CCFLAGS}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c ${OBJ_DIRS}
	${CC} $< -O$O -o $@ -c ${INCL_PATH_FL} ${CCFLAGS}

${OBJ_DIR}/%.cxx.o: ${SRC_DIR}/%.cpp ${OBJ_DIRS} ${OUT_JOLT}
	${CXX} $< -O$O -o $@ -c ${INCL_PATH_FL} ${CCFLAGS}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.m ${OBJ_DIRS}
	${CC} $< -O$O -o $@ -c ${INCL_PATH_FL} ${CCFLAGS}

${OBJ_DIR}/%.air: ${SHDR_SRC}/%.metal ${OBJ_DIR}
	xcrun metal -O$O -c -o $@ $<

${OUT_JOLT}:
	${MAKE} -C ${JOLT_DIR} -f ../../${SCRIPT_SRC} MAKE=${MAKE}

${SHDR_DIR}/default.metallib: ${SHDR_AIR_OUT} ${SHDR_DIR}
	xcrun metal -o $@ ${SHDR_AIR_OUT}

${RES_DIR}/%: ${RES}/% ${RES_DIR}
	@mkdir -p `dirname $@`
	cp -f $< $@

${OBJ_DIRS}: ${SRC} ${SRC_CXX}
	mkdir -p $@

${SHDR_DIR}:
	mkdir -p $@

${RES_DIR}:
	mkdir -p $@

${OUT_DIR}:
	mkdir $@

clean:
	rm -fr ${OUT_DIR}

fullclean: clean
	rm -fr ${OUT_JOLT_DIR}
