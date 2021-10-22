DEBUG := #-DDEBUG

LIB430COREDIR = ..

TEST_CSRC = test.c
TEST_OUT = $(patsubst %.c,%.o,$(TEST_CSRC))

all: $(LIB430COREDIR)/lib430core.a $(TEST_OUT)

$(LIB430COREDIR)/lib430core.a:
	@$(MAKE) -C ..

.c.o:
	@echo "Testing" $< "..."
	@msp430-elf32-gcc -T../link.ld $< -o tmp.elf -nostdlib -lgcc -lc
	@msp430-elf32-objcopy -Obinary tmp.elf tmp.bin
	@echo "#define TESTBIN \\" > test.h
	@od -t x1 tmp.bin | awk '{for (i=2; i<=NF; i++) printf "0x" $$i ", "}' >> test.h
	@printf "\n#define TESTSIZE %u \n" $$(msp430-elf32-objdump tmp.elf -Dj .text | grep -E "^\s+[0-9a-fA-F]+:" | wc -l) >> test.h
	@echo >> test.h
	@gcc -Os -ggdb -g3 -Wall -W -pedantic $(DEBUG) -I../src -o tmp \
		core_test.c -L$(LIB430COREDIR) -l430core
	@./tmp > log
	@rm log tmp.elf tmp.bin tmp test.h
