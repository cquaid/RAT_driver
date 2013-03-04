SRC = main.c RAT_driver.c
OBJ = ${SRC:.c=.o}

# gnu99 used to get rid of the implicit declairation of 'usleep' warning.
CFLAGS = -std=gnu99 -pedantic -Wall
LDFLAGS = -lusb -lX11 -lXext -lXtst

all: options RAT_driver

options:
	@echo RAT_driver build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

RAT_driver: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f RAT_driver ${OBJ}

.PHONY: all options clean
	
