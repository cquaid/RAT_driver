.SECONDARY:
.SECONDEXPANSION:

# Targets:
#   RAT7              - build RAT7 driver
#   Albino7           - build Albino7 driver
#
#   install_RAT7      - build and install RAT7 driver
#   install_Albino7   - build and install Albino7 driver
#
#   uninstall_RAT7    - uinstall RAT7 driver
#   uinstall_Albino7  - uinstall Albino7 driver

OBJDIR_BASE := $(CURDIR)/obj
BINDIR := $(CURDIR)/bin
INSTALL_DIR ?= /usr/local/bin

SHARED_SRC := main.c RAT_driver.c uinput.c
SHARED_OBJ := $(SHARED_SRC:.c=.o)

BIN_NAMES := RAT7 Albino7
BINS := $(foreach bin,$(BIN_NAMES),$(BINDIR)/$(bin))

BIN_RULES := $(BIN_NAMES)
INSTALL_RULES := $(foreach bin,$(BIN_NAMES),install_$(bin))
UNINSTALL_RULES := $(foreach bin,$(BIN_NAMES),uninstall_$(bin))

# Cyteck vendor id
VENDOR := 06a3

EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=

SHARED_LDFLAGS := -lusb

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
SHARED_CLFAGS := -std=gnu99 -pedantic -Wall
SHARED_CFLAGS += -DUINPUT_PATH='"/dev/uinput"' -DVENDOR_ID="0x$(VENDOR)"
# uncomment to kill the driver on Snipe (default profile only)
#SHARED_CFLAGS += -DKILL_ON_SNIPE

RAT7_PRODUCT := 0ccb
RAT7_CFLAGS  := -DRAT7 -DPRODUCT_ID="0x$(RAT7_PRODUCT)"

Albino7_PRODUCT := 0cce
Albino7_CFLAGS  := -DALBINO7 -DPRODUCT_ID="0x$(Albino7_PRODUCT)"


.PHONY: all
all: $(BINS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR_BASE) $(BINDIR)


.PHONY: $(INSTALL_RULES)
$(INSTALL_RULES): NAME = $(patsubst install_%,%,$@)
$(INSTALL_RULES): PRODUCT = $($(NAME)_PRODUCT)
$(INSTALL_RULES): BINARY = $(INSTALL_DIR)/$(NAME)
$(INSTALL_RULES): $(BINDIR)/$$(NAME)
	cp $< $(BINARY)
	$(CURDIR)/add-to-udev.sh $(VENDOR) $(PRODUCT) $(BINARY)
	#/etc/init.d/udev restart


.PHONY: $(UNINSTALL_RULES)
$(UNINSTALL_RULES): NAME = $(patsubst uninstall_%,%,$@)
$(UNINSTALL_RULES): PRODUCT = $($(NAME)_PRODUCT)
$(UNINSTALL_RULES): BINARY = $(INSTALL_DIR)/$(NAME)
$(UNINSTALL_RULES):
	rm -f /lib/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f /etc/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f $(BINARY)
	#/etc/init.d/udev restart

# Phony to force the $(CURDIR)/bin/$(NAME) rules easily
.PHONY: $(BIN_RULES)
$(BIN_RULES): $(BINDIR)/$$@

$(BINS): type = $(notdir $(basename $@))
$(BINS): OBJ = $(SHARED_OBJ) $($(type)_OBJ)
$(BINS): OBJDIR = obj/$(type)
$(BINS): CFLAGS = $(SHARED_CFLAGS) $($(type)_CFLAGS) $(EXTRA_CFLAGS)
$(BINS): LDFLAGS = $(SHARED_LDFLAGS) $($(type)_LDFLAGS) $(EXTRA_LDFLAGS)
$(BINS):
	@echo RAT_driver build options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@mkdir -p $(OBJDIR) $(dir $@)
	set -ex; for obj in $(OBJ) ; do \
		src=$$(basename $${obj} .o).c ; \
		$(CC) -c $(CFLAGS) $${src} -o $(OBJDIR)/$${obj} ; \
	done
	$(CC) -o $@ $(CFLAGS) $(foreach obj,$(OBJ),$(OBJDIR)/$(obj)) $(LDFLAGS)


