OUT_DIR := Linux_Debug
OUT := ${OUT_DIR}/libJolt.a

MAKE ?= make

${OUT}: ${OUT_DIR}
	${MAKE} -j8 -C ${OUT_DIR}
	./${OUT_DIR}/UnitTests

${OUT_DIR}:
	./cmake_linux_clang_gcc.sh Release
