# ==============================================================================
# Makefile raiz para Bomberman
# Delega en 2DGame/02-Bubble
# ==============================================================================

SUBDIR := 2DGame/02-Bubble

.PHONY: all run clean help test

all:
	@$(MAKE) -C $(SUBDIR) all

run:
	@$(MAKE) -C $(SUBDIR) run

test:
	@$(MAKE) -C $(SUBDIR) test

clean:
	@$(MAKE) -C $(SUBDIR) clean

help:
	@$(MAKE) -C $(SUBDIR) help
