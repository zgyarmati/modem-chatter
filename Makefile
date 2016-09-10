BIN = modem-chatter
CFLAGS = -g -lreadline


$(BIN): $(BIN).c
	$(CC)  -o $@ $< $(CFLAGS)
all: $(BIN)

clean:
	rm -f $(BIN) *~ *.o
