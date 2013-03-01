
CPPFLAGS=-I ./

all: eig 3-sites-linear/hamiltonian 3-sites-linear/mean-phonons 

clean:
	rm -f eig
	rm -f 3-sites-linear/hamiltonian
	rm -f 3-sites-linear/mean-phonons
	rm -rf 3-sites-linear/calculations
