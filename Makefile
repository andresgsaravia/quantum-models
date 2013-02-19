
CPPFLAGS=-I ./

all: eig mean-phonons

clean:
	rm -f eig
	rm -f mean-phonons
	rm -rf calculations
