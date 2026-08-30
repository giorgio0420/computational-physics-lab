# Left as written

`corr.c` is published unrepaired and is excluded from the default `make` target. The
specific defects are listed in the repository README, under "Left as written".

Correcting it would mean rewriting the accumulation loop and the period detection rather
than fixing a line, so it is kept here as a record instead of being silently repaired or
deleted. `periodo.c`, in `src/rossler/`, is the reliable period measurement.
