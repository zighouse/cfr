.PHONY: all clean

LIB_DIR := .libs
OBJ_DIR := .libs/objs
BIN_DIR := .

LIBS += $(LIB_DIR)/libcf.a
BINS += $(BIN_DIR)/cfr
BINS += $(BIN_DIR)/testcf

OBJS += $(OBJ_DIR)/cf.o
OBJS += $(OBJ_DIR)/homo.o
OBJS += $(OBJ_DIR)/bihomo.o
OBJS += $(OBJ_DIR)/approx.o
OBJS += $(OBJ_DIR)/numbs.o
OBJS += $(OBJ_DIR)/gmp.o
OBJS += $(OBJ_DIR)/gcf.o
OBJS += $(OBJ_DIR)/gendec.o

CFLAGS += -Wall -Iinclude
LDFLAGS += -lgmp

all: $(LIBS) $(BINS)

clean:
	rm -f $(BINS) $(LIBS) $(OBJS)

$(BIN_DIR)/cfr: source/cfr.c
	mkdir -p $(BIN_DIR)
	gcc -O2 -o $@ $^
	strip $@

$(LIB_DIR)/libcf.a: $(OBJS)
	mkdir -p $(LIB_DIR)
	ar Ur $@ $^ 

$(BIN_DIR)/testcf: source/testcf.c $(LIBS)
	mkdir -p $(BIN_DIR)
	gcc -O2 -o $@ $< -L$(LIB_DIR) -lcf $(LDFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.o: source/%.c
	mkdir -p $(OBJ_DIR)
	gcc -O2 -o $@ $(CFLAGS) -c $<

