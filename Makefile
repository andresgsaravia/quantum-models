
CC=g++
CPPFLAGS = -I ./lib/

all: 
	$(CC) $(CPPFLAGS) src/eig.cpp -o bin/eig
	mkdir bin/3-sites-linear
	$(CC) $(CPPFLAGS) src/3-sites-linear/hamiltonian.cpp -o bin/3-sites-linear/hamiltonian
	$(CC) $(CPPFLAGS) src/3-sites-linear/mean-phonons.cpp -o bin/3-sites-linear/mean-phonons
	$(CC) $(CPPFLAGS) src/3-sites-linear/splice-eigenvecs.cpp -o bin/3-sites-linear/splice-eigenvecs

clean:
	rm -rf bin/*
