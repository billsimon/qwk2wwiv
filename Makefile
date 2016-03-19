CC=cc
CFLAGS=-I.
DEPS = qwk2wwiv.h
OBJ = inih/ini.o w2r.o q2w.o main.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

qwk2wwiv: $(OBJ)
	$(CC) -o qwk2wwiv -o $@ $^ $(CFLAGS)
	
.PHONY: clean

clean:
	rm -f $(OBJ) qwk2wwiv 
