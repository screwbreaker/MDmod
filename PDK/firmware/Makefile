SDCC=sdcc
OUTDIR=build

FILENAME=pdk_mdmod

all: PFS154 PFS172 PFS173 PMS150C PMS152 PMS171B

PFS154:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPFS154 -mpdk14 -o $(OUTDIR)/$(FILENAME)_pfs154.ihx $(FILENAME).c

PFS172:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPFS172 -mpdk14 -o $(OUTDIR)/$(FILENAME)_pfs172.ihx $(FILENAME).c

PFS173:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPFS173 -mpdk15 -o $(OUTDIR)/$(FILENAME)_pfs173.ihx $(FILENAME).c

PMS150C:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPMS150C -mpdk13 -o $(OUTDIR)/$(FILENAME)_pms150c.ihx $(FILENAME).c

PMS152:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPMS152 -mpdk14 -o $(OUTDIR)/$(FILENAME)_pms152.ihx $(FILENAME).c

PMS171B:
	mkdir -p $(OUTDIR)
	$(SDCC) -DPMS171B -mpdk14 -o $(OUTDIR)/$(FILENAME)_pms171b.ihx $(FILENAME).c

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
