
CC = gcc
NT_PATH = /opt/napatech3
CFLAGS = -I$(NT_PATH)/include -L$(NT_PATH)/lib -lntapi -lntos

ntptpmon:
	$(CC) $(CFLAGS) -o ntptpmon ntptpmon.c
