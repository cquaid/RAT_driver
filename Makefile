SRC = main.c RAT_driver.c
OBJ = ${SRC:.c=.o}

#CFLAGS = -pedantic -Wall
CFLAGS =
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
	
