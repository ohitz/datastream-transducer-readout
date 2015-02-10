
CC		= gcc
CFLAGS		= -Wall

OBJS		= dsreadout.o \
		  serial.o \
		  string.o \
		  transducer.o

dsreadout:	$(OBJS)

clean:		
		-rm $(OBJS) dsreadout
