SRC = main.c RAT_driver.c key_events.c
OBJ = ${SRC:.c=.o}

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
CFLAGS = -std=gnu99 -pedantic -Wall
LDFLAGS = -lusb -lX11 -lXext -lXtst
OPTIONS =
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

RAT7: OPTIONS += -DRAT7
RAT7: options ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

Albino7: OPTIONS += -DALBINO7
Albino7: options ${OBJ}
	@echo CC -o $@
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f RAT7 Albino7 ${OBJ}

.PHONY: all options clean
	
