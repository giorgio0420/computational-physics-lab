CC      ?= cc
CFLAGS  ?= -O2 -Wall
LDLIBS  := -lm
BIN     := bin

ROSSLER    := integratore periodo duratatransiente corr ex1 ex2
STOCHASTIC := rw rw2d ale per gas

ROSSLER_BIN    := $(addprefix $(BIN)/,$(ROSSLER))
STOCHASTIC_BIN := $(addprefix $(BIN)/,$(STOCHASTIC))

.PHONY: all rossler stochastic check figures clean

all: rossler stochastic

rossler:    $(ROSSLER_BIN)
stochastic: $(STOCHASTIC_BIN)

$(BIN)/%: src/rossler/%.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

$(BIN)/%: src/stochastic/%.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

$(BIN):
	mkdir -p $(BIN)

# Animations. Because these are real file targets, make only rebuilds a GIF
# when its program or its script is newer than it - not on every invocation.
FIGURES := figures/gas.gif figures/percolation.gif

figures: $(FIGURES)

figures/gas.gif: $(BIN)/gas scripts/animate_gas.py
	python scripts/animate_gas.py

figures/percolation.gif: $(BIN)/per scripts/animate_per.py
	python scripts/animate_per.py

# The three-density comparison is a one-off for the README, not part of `figures`.
figures/gas_densities.gif: $(BIN)/gas scripts/animate_gas.py
	python scripts/animate_gas.py --compare

# Reproduce the three headline numbers from the report.
check: all
	@sh scripts/check.sh

clean:
	rm -rf $(BIN)
