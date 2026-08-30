CC      ?= cc
CFLAGS  ?= -O2 -Wall
LDLIBS  := -lm
BIN     := bin

ROSSLER    := integratore periodo duratatransiente ex1 ex2
STOCHASTIC := rw rw2d ale per gas

ROSSLER_BIN    := $(addprefix $(BIN)/,$(ROSSLER))
STOCHASTIC_BIN := $(addprefix $(BIN)/,$(STOCHASTIC))

.PHONY: all rossler stochastic check clean

all: rossler stochastic

rossler:    $(ROSSLER_BIN)
stochastic: $(STOCHASTIC_BIN)

$(BIN)/%: src/rossler/%.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

$(BIN)/%: src/stochastic/%.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

$(BIN):
	mkdir -p $(BIN)

# Reproduce the three headline numbers from the report.
check: all
	@sh scripts/check.sh

clean:
	rm -rf $(BIN)
