
CPPFLAGS=-I ./

all: eig 3-sites-linear/hamiltonian 3-sites-linear/mean-phonons 3-sites-linear/splice-eigenvecs

clean:
	rm -f eig
	rm -f 3-sites-linear/hamiltonian
	rm -f 3-sites-linear/mean-phonons
	rm -f 3-sites-linear/splice-eigenvecs
	rm -rf 3-sites-linear/calculations
