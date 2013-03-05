SRC = main.c RAT_driver.c
OBJ = ${SRC:.c=.o}

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
CFLAGS = -std=gnu99 -pedantic -Wall
LDFLAGS = -lusb -lX11 -lXext -lXtst
OPTIONS =
# uncomment for the Albino R.A.T. 7
#OPTIONS += -DALBINO7
# uncomment to kill the driver on Snipe (default profile only)
#OPTIONS += -DKILL_ON_SNIPE

all: options RAT_driver

options:
	@echo RAT_driver build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC} ${OPTIONS}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} ${OPTIONS} $<

RAT_driver: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f RAT_driver ${OBJ}

.PHONY: all options clean
	
