SHELL := /bin/bash

GBDK_HOME	= /opt/gbdk/
HUGE_HOME = /opt/hUGEDriver-6/
# GBDK_HOME	= /mnt/c/Users/smark/gbdk/linux64/
# HUGE_HOME = /mnt/c/Users/smark/gbdk/hUGEDriver-6.1.3/

LCC	= $(GBDK_HOME)bin/lcc

# These can be compiled individually as
# "make gb" and "make gb-clean"
# "make pocket" and "make pocket-clean"
TARGETS = gb pocket

LCCFLAGS_gb = -Wa-l -Wl-m -Wm-yoA -Wm-yt0x1B -Wf-bo1 -Wf-bo2 -Wb-ext=.rel -Wm-yS
LCCFLAGS_pocket = $(LCCFLAGS_gb) 

LCCFLAGS += $(LCCFLAGS_$(EXT))

PROJECT_NAME = PGIS

SRCDIR = src
OBJDIR = obj/$(EXT)
BINDIR = build/$(EXT)
MKDIRS = $(OBJDIR) $(BINDIR)

HUGE_LIB = $(HUGE_HOME)gbdk/hUGEDriver.lib
HUGE_H = $(HUGE_HOME)include/

BINS = $(OBJDIR)/$(PROJECT_NAME)_$(VERSION).$(EXT)
CSOURCES = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.c)))
OBJS  = $(CSOURCES:%.c=$(OBJDIR)/%.o)

.phony: all clean
# Builds all targets sequentially
# "make all"
all: $(TARGETS)

# Compule .c files in "src/" to .o object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(LCC) $(CFLAGS) -c -I$(HUGE_H) -I$(GBDK_HOME) -o $@ $<

# Link the compiled object files into a .$(EXT) ROM file
$(BINS): $(OBJS)
	$(LCC) $(LCCFLAGS) $(CFLAGS) -o $(BINDIR)/$(PROJECT_NAME)_$(VERSION).$(EXT) $(OBJS) -Wl-l$(HUGE_LIB)

clean:
	@echo Cleaning
	@for target in $(TARGETS); do \
				$(MAKE) $$target-clean; \
	done

# Include build targets
include Makefile.targets

# Create directories
ifneq ($(strip $(EXT)),)
$(info $(shell mkdir -p $(MKDIRS)))
endif