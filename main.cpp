#include <iostream>
#include <cmath>
#include <Eigen/Dense>


using namespace std;
using namespace Eigen;

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

  MatrixXf h(size,size);

  // initializing the hamiltonian at zeros
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      h(row, col) = 0;
    }
  }
    

  // building the hamiltonian
  // I make a list (h_list) with triplets (row, col, value) of all
  // the non-zero elements. If there are (row, col) pairs repeated
  // their values will be summed.
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {

	  // band energies
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = band_energy[e1 - 1] + band_energy[e2 - 1];
	  h(row,col) += energy;

	  // on-site Coulomb repulsion
	  if (e1 == e2) {
	    row = state_label(e1, e1, ir, ram, ir_phonons);
	    col = state_label(e1, e1, ir, ram, ir_phonons);
	    h(row, col) += on_site_repulsion;
	  }

	  // nearest-neighbor hopping
	  if (e1 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 + 1, e2, ir, ram, ir_phonons);
	    h(row, col) += nn_hopping;
	  }
	  if (e1 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 - 1, e2, ir, ram, ir_phonons);
	    h(row, col) += nn_hopping;
	  }
	  if (e2 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 + 1, ir, ram, ir_phonons);
	    h(row, col) += nn_hopping;
	  }
	  if (e2 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 - 1, ir, ram, ir_phonons);
	    h(row, col) += nn_hopping;
	  }
	  
	  // infrared phonons energy
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ir * ir_energy;
	  h(row, col) += energy;

	  // raman phonons energy
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ram * raman_energy;
	  h(row, col) += energy;

	  // electron - infrared phonons interaction
	  n = e1 + e2 - 4;
	  if (ir != ir_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir + 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir + 1);
	    h(row, col) += energy;
	  }
	  if (ir != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir - 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir);
	    h(row, col) += energy;
	  }
	  
	  // electron - Raman phonons interaction
	  n = abs(e1 - 2) + abs(e2 - 2) - raman_shift;
	  if (ram != ir_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram + 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram + 1);
	    h(row, col) += energy;
	  }
	  if (ram != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram - 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram);
	    h(row, col) += energy;
	  }
	}
      }
    }
  }

  SelfAdjointEigenSolver<MatrixXf> eigensolver(h);
  VectorXf eigenvalues = eigensolver.eigenvalues();

  int how_many;
  cout << "How many eigenvalues should I print?" << endl;
  cin >> how_many;

  
  for (n = 0; n < how_many; n++) {
    cout << eigenvalues(n) << endl;
  }
  return 0;
}
