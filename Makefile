C_COMPILER=clang

CFLAGS = -std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wno-misleading-indentation
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
CFLAGS += -Werror

INC_DIRS = -I${XILINX_VIVADO}/data/xsim/include -Iinclude

all:
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) test.c

format:
	clang-format -i test.c test/tests.c osi.h

lint:
	clang-tidy test.c -header-filter=.* -- $(CFLAGS) $(INC_DIRS)

docs:
	doxygen

test1:
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) -Idependencies/Unity/src/ -g dependencies/Unity/src/unity.c test/tests.c