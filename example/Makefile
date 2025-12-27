CC            :=gcc
AGENDA_HEADER :=../agenda.h
FILES         := example.c example_2.c example_2.h
CFLAGS        :=-Wall -Wextra -ggdb

example: example.c example_2.o
	$(CC) $(CFLAGS) $(AGENDA_INCLUDE) $^ -o $@

example_2.o: example_2.c example_2.h $(AGENDA_HEADER)
	$(CC) $(CFLAGS) $(AGENDA_INCLUDE) $^ -c

.PHONY: clean
clean:
	find . -type f -name "*.gch" -delete
	cd .. && find . -type f -name "*.gch" -delete
	find . -type f -name "*.o" -delete
	rm -f "./example" "./example.exe"

../TAGS:
	cd .. && (find . -type f -name *.[ch] | etags -)
