CC=gcc
CFLAGS= -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
SRCS= src/main.c src/config/config.c src/daemon/daemon.c src/http/http.c src/server/server.c src/utils/string/string.c

OBJS=$(SRCS:.c=.o)

httpd: ${OBJS}
	${CC} -o $@ ${OBJS}

check: httpd
	@echo "checking....."
	@echo "ALL GOOD "

clean:
	${RM} httpd ${OBJS}
