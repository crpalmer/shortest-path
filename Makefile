sp: sp.o pq.o
	$(CC) sp.o pq.o -o sp

sp.o: sp.h
pq.o: sp.h
