GCC=gcc-4.7
LD=gold

FLAGS = -Werror -Wall -Wextra #-Wpedantic

HEADERS = schema.h parser.h b+tree.h
MAIN = main.c d.o
OBJ = parser.o pager.o b+tree.o select.o

d: $(MAIN) $(HEADERS)
	$(GCC) $(MAIN) -o $@ $(FLAGS)

# "make t" to speedup development
t: d
	./d test.db

# demonstration of incremental linking, no real purpose
d.o: $(OBJ)
	$(LD) -i $^ -o $@

# object files
parser.o: parser.c parser.h schema.h
	$(GCC) -c $< $(FLAGS)

pager.o: pager.c
	$(GCC) -c $< $(FLAGS)

b+tree.o: b+tree.c
	$(GCC) -c $< $(FLAGS)

select.o: select.c schema.h
	$(GCC) -c select.c $(FLAGS)

# regression tests
test: d
	python test.py

clean:
	rm -f d a.out *.o *.db

.PHONY: t clean test

