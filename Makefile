# ==============================================================================
# Makefile raiz para Bomberman
# Delega en 2DGame/02-Bubble
# ==============================================================================

SUBDIR := 2DGame/02-Bubble

.PHONY: all run clean help test web

all:
	@$(MAKE) -C $(SUBDIR) all

run:
	@$(MAKE) -C $(SUBDIR) run

test:
	@$(MAKE) -C $(SUBDIR) test

web:
	@$(MAKE) -C $(SUBDIR) web

clean:
	@$(MAKE) -C $(SUBDIR) clean

help:
	@$(MAKE) -C $(SUBDIR) help
