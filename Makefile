.PHONY: all clean

LIB_DIR := .libs
OBJ_DIR := .libs/objs
BIN_DIR := .

LIBS += $(LIB_DIR)/libcf.a
BINS += $(BIN_DIR)/cfr
BINS += $(BIN_DIR)/testcf
BINS += $(BIN_DIR)/pi

OBJS += $(OBJ_DIR)/cf.o
OBJS += $(OBJ_DIR)/homo.o
OBJS += $(OBJ_DIR)/bihomo.o
OBJS += $(OBJ_DIR)/bihomo_mpz.o
OBJS += $(OBJ_DIR)/integer.o
OBJS += $(OBJ_DIR)/converg.o
OBJS += $(OBJ_DIR)/numbs.o
OBJS += $(OBJ_DIR)/gmp.o
OBJS += $(OBJ_DIR)/gcf.o
OBJS += $(OBJ_DIR)/gendec.o
OBJS += $(OBJ_DIR)/float.o

CFLAGS += -Wall -Iinclude
LDFLAGS += -lgmp -lm
#OPTS = -O2 -g
OPTS = -O0 -g

all: $(LIBS) $(BINS)

clean:
	rm -f $(BINS) $(LIBS) $(OBJS)

$(BIN_DIR)/cfr: source/cfr.c
	mkdir -p $(BIN_DIR)
	gcc $(OPTS) -o $@ $^ -L$(LIB_DIR) -lcf $(LDFLAGS) $(CFLAGS)

$(LIB_DIR)/libcf.a: $(OBJS)
	mkdir -p $(LIB_DIR)
	ar Ur $@ $^ 

$(BIN_DIR)/testcf: test/testcf.c $(LIBS)
	mkdir -p $(BIN_DIR)
	gcc $(OPTS) -o $@ $< -L$(LIB_DIR) -lcf $(LDFLAGS) $(CFLAGS)

$(BIN_DIR)/pi: test/pi.c $(LIBS)
	mkdir -p $(BIN_DIR)
	gcc $(OPTS) -o $@ $< -L$(LIB_DIR) -lcf $(LDFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.o: source/%.c
	mkdir -p $(OBJ_DIR)
	gcc $(OPTS) -o $@ $(CFLAGS) -c $<

