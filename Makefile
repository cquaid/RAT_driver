SRC = main.c RAT_driver.c uinput.c
OBJ = ${SRC:.c=.o}

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
CFLAGS = -std=gnu99 -pedantic -Wall
LDFLAGS = -lusb
VENDOR = 06a3
OPTIONS = -DUINPUT_PATH='"/dev/uinput"' -DVENDOR_ID="(0x${VENDOR})"
# uncomment to kill the driver on Snipe (default profile only)
#OPTIONS += -DKILL_ON_SNIPE

all: options RAT7

options:
	@echo RAT_driver build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC} ${OPTIONS}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} ${OPTIONS} $<

RAT7: PRODUCT = 0ccb
RAT7: OPTIONS += -DRAT7 -DPRODUCT_ID="(0x${PRODUCT})"
RAT7: options ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

Albino7: PRODUCT = 0cce
Albino7: OPTIONS += -DALBINO7 -DPRODUCT_ID="(0x${PRODUCT})"
Albino7: options ${OBJ}
	@echo CC -o $@
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f RAT7 Albino7 ${OBJ}

uninstall_Albino7: PRODUCT = 0cce
uninstall_Albino7:
	@echo uninstalling
	@rm -f /{lib,etc}/udev/rules.d/81-local-${VENDOR}-${PRODUCT}.rules
	@rm -f /usr/bin/Albino7
	@echo restarting udev
	@/etc/init.d/udev restart

install_Albino7: PRODUCT = 0cce
install_Albino7:
	@echo installing
	@cp Albino7 /usr/bin
	@./add-to-udev.sh ${VENDOR} ${PRODUCT} /usr/bin/Albino7
	@echo restarting udev
	@/etc/init.d/udev restart


uninstall_RAT7: PRODUCT = 0ccb
uninstall_RAT7:
	@echo uninstalling
	@rm -f /{lib,etc}/udev/rules.d/81-local-${VENDOR}-${PRODUCT}.rules
	@rm -f /usr/bin/RAT7
	@echo restarting udev
	@/etc/init.d/udev restart

install_RAT7: PRODUCT = 0ccb
install_RAT7:
	@echo installing
	@cp RAT7 /usr/bin
	@./add-to-udev.sh ${VENDOR} ${PRODUCT} /usr/bin/RAT7
	@echo restarting udev
	@/etc/init.d/udev restart

.PHONY: all options clean install_Albino7 uninstall_Albino7 install_RAT7 uninstall_RAT7
