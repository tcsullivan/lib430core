CROSS :=
CC := $(CROSS)gcc
CFLAGS := -Os -ggdb -g3 \
		  -Wall -Wextra -pedantic -Werror

CSRC := $(wildcard src/*.c)
COBJ := $(patsubst %.c,%.o,$(CSRC))

all: lib430core.a

clean:
	@echo "  CLEAN"
	@rm -f lib430core.a $(COBJ)

lib430core.a: $(COBJ)
	@echo "  AR      " $@
	@$(CROSS)ar rc $@ $(COBJ)
	@$(CROSS)size -t $@

.c.o:
	@echo "  CC      " $<
	@$(CC) $(CFLAGS) -c $< -o $@

