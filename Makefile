# ==============================================================================
# Makefile raiz para Bomberman
# Delega en 2DGame/02-Bubble
# ==============================================================================

SUBDIR := 2DGame/02-Bubble

.PHONY: all run clean help

all:
	@$(MAKE) -C $(SUBDIR) all

run:
	@$(MAKE) -C $(SUBDIR) run

clean:
	@$(MAKE) -C $(SUBDIR) clean

help:
	@$(MAKE) -C $(SUBDIR) help
