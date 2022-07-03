SDCC=sdcc
OUTDIR=build
FAMILY=pic14
PROC=16f630

FILENAME=pic_mdmod

all: 16f630

16f630:
	$(SDCC) --use-non-free --no-extended-instructions -m$(FAMILY) -p$(PROC) $(FILENAME).c

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
