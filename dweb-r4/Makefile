CC = gcc
OBJS = socket.o http.o
CFLAGS = -g -D__USE_FORK__
LIBS = -lpthread


dweb: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(OBJS)
