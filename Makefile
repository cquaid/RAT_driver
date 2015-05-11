.SECONDARY:
.SECONDEXPANSION:

SHARED_SRC = main.c RAT_driver.c uinput.c
SHARED_OBJ = $(SHARED_SRC:.c=.o)

BIN_NAMES = RAT7 Albino7
BINS = $(foreach bin,$(BIN_NAMES),$(BINDIR)/$(bin))

OBJDIR_BASE = $(CURDIR)/obj
BINDIR = $(CURDIR)/bin

# Cyteck vendor id
VENDOR = 06a3

SHARED_LDFLAGS = -lusb

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
SHARED_CLFAGS  = -std=gnu99 -pedantic -Wall
SHARED_CFLAGS += -DUINPUT_PATH='"/dev/uinput"' -DVENDOR_ID="0x$(VENDOR)"
# uncomment to kill the driver on Snipe (default profile only)
#SHARED_CFLAGS += -DKILL_ON_SNIPE


RAT7_CFLAGS = -DRAT7 -DPRODUCT_ID="0x0ccb"
Albino7_CFLAGS = -DALBINO7 -DPRODUCT_ID="0x0cce"

.PHONY: all
all: $(BINS)

.PHONY: clean
clean:
	rm -f $(OBJDIR_BASE) $(BINDIR)

.PHONY: uninstall_Albino7
uninstall_Albino7: PRODUCT = 0cce
uninstall_Albino7:
	rm -f /lib/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f /etc/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f /usr/bin/Albino7
	/etc/init.d/udev restart

.PHONY: uninstall_RAT7
uninstall_RAT7: PRODUCT = 0ccb
uninstall_RAT7:
	rm -f /lib/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f /etc/udev/rules.d/81-local-$(VENDOR)-$(PRODUCT).rules
	rm -f /usr/bin/RAT7
	/etc/init.d/udev restart

.PHONY: install_Albino7
install_Albino7: PRODUCT = 0cce
install_Albino7: $(BINDIR)/Albino7
	cp $< /usr/bin
	$(CURDIR)/add-to-udev.sh $(VENDOR) $(PRODUCT) /usr/bin/Albino7
	/etc/init.d/udev restart

.PHONY: install_RAT7
install_RAT7: PRODUCT = 0ccb
install_RAT7: $(BINDIR)/RAT7
	cp $< /usr/bin
	$(CURDIR)/add-to-udev.sh $(VENDOR) $(PRODUCT) /usr/bin/RAT7
	/etc/init.d/udev restart

$(BINS): type = $(notdir $(basename $@))
$(BINS): OBJ = $(SHARED_OBJ) $($(type)_OBJ)
$(BINS): OBJDIR = obj/$(type)
$(BINS): CFLAGS = $(SHARED_CFLAGS) $($(type)_CFLAGS)
$(BINS): LDFLAGS = $(SHARED_LDFLAGS) $($(type)_LDFLAGS)
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


