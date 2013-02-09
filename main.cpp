#include <iostream>
#include <cmath>
#include "Eigen/Sparse"

typedef Eigen::SparseMatrix<double> SpMat;   // column-major sparse matrix of type double
typedef Eigen::Triplet<double> T;            // a triplet specifies a non-zero element of a sparse matrix

using namespace std;

// Assing a unique label according to the position of electron 1 (e1), electron 2 (e2),
// the number if infrared and Raman phonons (ir and ram) and the total number of infrared
// phonons (n_ir)
int state_label (int e1, int e2, int ir, int ram, int n_ir)
{
  return e1 + (3 * (e2 -1)) + (9 * ir) + (9 * ram * (n_ir + 1)) - 1;
}


int main (void) {
  int ir_phonons, raman_phonons, size;
  double band_energy[3];
  double nn_hopping, on_site_repulsion, ir_energy, raman_energy, raman_shift;
  double e_ir_coupling, e_ram_coupling;
  vector<T> h_list;     // list of non-zero elements of the hamiltonian (as Triplets)

  int e1, e2, ir, ram, row, col, n;  // just counters
  double energy;  // useful variable to fill in the hamiltonian elements

  cout << endl << "This program calculates the eigenvalues for a model hamiltonian" << endl;
  cout << "representing a linear cluster of 3 atoms with 2 electrons " << endl;
  cout << "hopping between them." << endl << endl;

  // TODO: Assert that the values make sense
  cout << "Please enter the following parameters:" << endl;
  cout << "Band energy for site 1: ";
  cin >> band_energy[0];
  cout << "Band energy for site 2: ";
  cin >> band_energy[1];
  cout << "Band energy for site 3: ";
  cin >> band_energy[2];
  cout << "Nearest neighbor hopping: ";
  cin >> nn_hopping;
  cout << "On site Coulomb repulsion: ";
  cin >> on_site_repulsion;
  cout << "Infrared phonon's energy: ";
  cin >> ir_energy;
  cout << "Electron - infrared phonons coupling: ";
  cin >> e_ir_coupling;
  cout << "Raman phonon's energy: ";
  cin >> raman_energy;
  cout << "Electron - raman phonons coupling: ";
  cin >> e_ram_coupling;
  cout << "Raman shift: ";
  cin >> raman_shift;
  cout << "Number of infrared phonons: ";
  cin >> ir_phonons;
  cout << "Number of Raman phonons: ";
  cin >> raman_phonons;

  size = 9 * (1 + raman_phonons) * (1 + ir_phonons);
  cout << "The size of the hamiltonian is: " << size << "x" << size << endl;

  // band energy interaction
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = band_energy[e1 - 1] + band_energy[e2 - 1];
	  h_list.push_back(T(row, col, energy));
	}
      }
    }
  }

  // on-site Coulomb repulsion
  for (e1 = 1; e1 <= 3; e1++) {
    for (ir = 0; ir <= ir_phonons; ir++) {
      for (ram = 0; ram <= raman_phonons; ram++) {
	row = state_label(e1, e1, ir, ram, ir_phonons);
	col = state_label(e1, e1, ir, ram, ir_phonons);
	h_list.push_back(T(row, col, on_site_repulsion));
      }
    }
  }	  

  // nearest-neighbor hopping
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  if (e1 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 + 1, e2, ir, ram, ir_phonons);
	    h_list.push_back(T(row, col, nn_hopping));
	  }
	  if (e1 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 - 1, e2, ir, ram, ir_phonons);
	    h_list.push_back(T(row, col, nn_hopping));
	  }
	  if (e2 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 + 1, ir, ram, ir_phonons);
	    h_list.push_back(T(row, col, nn_hopping));
	  }
	  if (e2 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 - 1, ir, ram, ir_phonons);
	    h_list.push_back(T(row, col, nn_hopping));
	  }
	}
      }
    }
  }

  // infrared phonons energy
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ir * ir_energy;
	  h_list.push_back(T(row, col, energy));
	}
      }
    }
  }

  // raman phonons energy
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ram * raman_energy;
	  h_list.push_back(T(row, col, energy));
	}
      }
    }
  }


  // electron - infrared phonons interaction
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  n = e1 + e2 - 4;
	  if (ir != ir_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir + 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir + 1);
	    h_list.push_back(T(row, col, energy));
	  }
	  if (ir != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir - 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir);
	    h_list.push_back(T(row, col, energy));
	  }
	}
      }
    }
  }

  // electron - Raman phonons interaction
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {
	  n = abs(e1 - 2) + abs(e2 - 2) - raman_shift;
	  if (ram != ir_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram + 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram + 1);
	    h_list.push_back(T(row, col, energy));
	  }
	  if (ram != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram - 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram);
	    h_list.push_back(T(row, col, energy));
	  }
	}
      }
    }
  }


  SpMat hamiltonian(size, size);
  hamiltonian.setFromTriplets(h_list.begin(), h_list.end());
  cout << "The matrix has a size of:" << hamiltonian.size() << endl;

  return 0;
}
