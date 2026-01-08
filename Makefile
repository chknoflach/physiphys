CC := gcc

SRCDIR := src
INCDIR := include
BUILD  := build
OBJDIR := $(BUILD)/obj

BIN := $(BUILD)/physi

SRC := $(sort $(shell find $(SRCDIR) -type f -name '*.c'))
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

DEP := $(OBJ:.o=.d)

CFLAGS  := -std=c99 -I$(INCDIR)
LDFLAGS := -L/usr/local/lib
LDLIBS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

WARNFLAGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
DBGFLAGS  := -g
SANFLAGS  := -fsanitize=address,undefined,leak -fno-omit-frame-pointer
# SANFLAGS += -fno-sanitize-recover=all

all: debug

re: fclean all

debug: CFLAGS  += $(DBGFLAGS) $(WARNFLAGS) $(SANFLAGS)
debug: LDFLAGS += $(SANFLAGS)
debug: $(BIN)

prod: CFLAGS += -O2 -DNDEBUG
prod: fclean $(BIN)

# Link
$(BIN): $(OBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Compile
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(BIN)

fclean:
	rm -rf $(BUILD)

# Include auto-generated dependencies (safe if none exist yet)
-include $(DEP)

.PHONY: all debug prod clean fclean re
