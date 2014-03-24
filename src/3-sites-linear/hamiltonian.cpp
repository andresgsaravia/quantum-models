/*
  This routine builds a hamiltonian and writes it to a file named "hamiltonian.mtx" for a model
  of a 3-atomic linear molecule with 2 vibrational modes (phonons) and two electrons (or holes) 
  hopping between the atoms. The inputs are read from the file "parameters.inp" if this file is 
  not found it will create a template for it. 

  The quantum model used is as follows:

  H = H_{el} + H_{ph} + H_{el-ph}

  where each contribution is:
  
  H_{el} = \sum_n \epsilon_n \rho_n + t\sum_{\langle nn'\rangle\sigma}(c_{n\sigma}^\dagger c_{n'\sigma} + H.c.) 
           + U\sum_n \rho_{n\downarrow}\rho_{n\uparrow}

  H_{ph} = \hbar \omega_{ir}a_{ir}^\dagger a_{ir} + \hbar \omega_R a_R^\dagger a_R

  H_{el-ph} = \lambda_{ir}(a_{ir} + a_{ir}^\dagger)(\rho_3 - \rho_1) + \lambda_R (a_R + a_R^\dagger)(\rho_1 + \rho_3-s_0)

  Reference: Physical Review B, 49, 3671–3674. doi:10.1103/PhysRevB.49.3671

  See the LICENSE file for information on how to reuse this code.
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <Eigen/Sparse>
#include <unsupported/Eigen/SparseExtra>
#include <vector>

Eigen::IOFormat LongPrinting(20);

void save_parameters(double*, double, double, double, double, double, double, double, int, int);

// Assigning a unique label according to the position of electron 1 (e1), electron 2 (e2),
// the number if infrared and Raman phonons (ir and ram) and the total number of infrared
// phonons (n_ir)
int state_label (int e1, int e2, int ir, int ram, int n_ir)
{
  return e1 + (3 * (e2 -1)) + (9 * ir) + (9 * ram * (n_ir + 1)) - 1;
}

typedef Eigen::SparseMatrix<double> SpMat;
typedef Eigen::Triplet<double> T;

int main (int argc, char *argv[]) {
  int ir_phonons, raman_phonons, size;
  double band_energy[3];
  double nn_hopping, on_site_repulsion, ir_energy, raman_energy, raman_shift;
  double e_ir_coupling, e_ram_coupling;

  int e1, e2, ir, ram, row, col, n;  // just counters
  double energy;  // useful variable to fill in the hamiltonian elements

  std::ifstream inputfile;
  std::string line;
  inputfile.open("parameters.inp");
  if (inputfile.is_open()) {
    // TODO: Assert that the values make sense
    getline(inputfile, line, ',');
    band_energy[0] = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    band_energy[1] = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    band_energy[2] = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    nn_hopping = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    on_site_repulsion = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    ir_energy = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    e_ir_coupling  = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    raman_energy = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    e_ram_coupling = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    raman_shift = atof(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    ir_phonons = atoi(line.c_str());
    getline(inputfile, line);

    getline(inputfile, line, ',');
    raman_phonons = atoi(line.c_str());
    getline(inputfile, line);

    inputfile.close();
  }
  else {
    std::cout << "Input file \"parameters.inp\" not found. I will create a template for you. " << std::endl;
    double temp[3] = {0, 0, 0};
    save_parameters(temp, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0);
    return 0;
  }

  size = 9 * (1 + raman_phonons) * (1 + ir_phonons);
  std::cout << "The size of the hamiltonian is: " << size << "x" << size << std::endl;

  std::vector<T> coeffsList;    // non-zero triplets
  
  // Filling the coefficients' vector
  for (e1 = 1; e1 <= 3; e1++) {
    for (e2 = 1; e2 <= 3; e2++) {
      for (ir = 0; ir <= ir_phonons; ir++) {
	for (ram = 0; ram <= raman_phonons; ram++) {

	  // band energies
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = band_energy[e1 - 1] + band_energy[e2 - 1];
	  coeffsList.push_back(T(row ,col, energy));

	  // on-site Coulomb repulsion
	  if (e1 == e2) {
	    row = state_label(e1, e1, ir, ram, ir_phonons);
	    col = state_label(e1, e1, ir, ram, ir_phonons);
	    coeffsList.push_back(T(row, col, on_site_repulsion));
	  }

	  // nearest-neighbor hopping
	  if (e1 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 + 1, e2, ir, ram, ir_phonons);
	    coeffsList.push_back(T(row, col, nn_hopping));
	  }
	  if (e1 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1 - 1, e2, ir, ram, ir_phonons);
	    coeffsList.push_back(T(row, col, nn_hopping));
	  }
	  if (e2 != 3) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 + 1, ir, ram, ir_phonons);
	    coeffsList.push_back(T(row, col, nn_hopping));
	  }
	  if (e2 != 1) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2 - 1, ir, ram, ir_phonons);
	    coeffsList.push_back(T(row, col, nn_hopping));
	  }
	  
	  // infrared phonons energy
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ir * ir_energy;
	  coeffsList.push_back(T(row, col, energy));

	  // raman phonons energy
	  row = state_label(e1, e2, ir, ram, ir_phonons);
	  col = state_label(e1, e2, ir, ram, ir_phonons);
	  energy = ram * raman_energy;
	  coeffsList.push_back(T(row, col, energy));

	  // electron - infrared phonons interaction
	  n = e1 + e2 - 4;
	  if (ir != ir_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir + 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir + 1);
	    coeffsList.push_back(T(row, col, energy));
	  }
	  if (ir != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir - 1, ram, ir_phonons);
	    energy = n * e_ir_coupling * sqrt(ir);
	    coeffsList.push_back(T(row, col, energy));
	  }
	  
	  // electron - Raman phonons interaction
	  n = abs(e1 - 2) + abs(e2 - 2) - raman_shift;
	  if (ram != raman_phonons) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram + 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram + 1);
	    coeffsList.push_back(T(row, col, energy));
	  }
	  if (ram != 0) {
	    row = state_label(e1, e2, ir, ram, ir_phonons);
	    col = state_label(e1, e2, ir, ram - 1, ir_phonons);
	    energy = n * e_ram_coupling * sqrt(ram);
	    coeffsList.push_back(T(row, col, energy));
	  }
	}
      }
    }
  }

  Eigen::SparseMatrix<double> h(size,size);
  h.setFromTriplets(coeffsList.begin(), coeffsList.end());

  std::cout << "Saving the hamiltonian matrix at \"hamiltonian.mtx\"... ";
  Eigen::saveMarket(h, "hamiltonian.mtx");
  std::cout << "Done." << std::endl;

  return 0;
}


void save_parameters(double *band_energy, double nn_hopping, double on_site_repulsion,
		     double ir_energy, double e_ir_coupling, double raman_energy, 
		     double e_ram_coupling, double raman_shift, int ir_phonons, int raman_phonons) {
  std::ofstream paramfile;
  paramfile.open("parameters.inp", std::ios::out);
  if (paramfile.is_open()) {
    paramfile << band_energy[0] << ", Band energy for site 1" << std::endl;
    paramfile << band_energy[1] << ", Band energy for site 2" << std::endl;
    paramfile << band_energy[2] << ", Band energy for site 3" << std::endl;
    paramfile << nn_hopping << ", Nearest neighbor hopping" << std::endl;
    paramfile << on_site_repulsion << ", On site Coulomb repulsion" << std::endl;
    paramfile << ir_energy << ", Infrared phonon's energy" << std::endl;
    paramfile << e_ir_coupling << ", Electron - infrared phonons coupling" << std::endl;
    paramfile << raman_energy << ", Raman phonon's energy" << std::endl;
    paramfile << e_ir_coupling << ", Electron - raman phonons coupling" << std::endl;
    paramfile << raman_shift << ", Raman shift" << std::endl;
    paramfile << ir_phonons << ", Number of infrared phonons" << std::endl;
    paramfile << raman_phonons << ", Number of raman phonons" << std::endl;
    paramfile.close();
  }
  else {
    std::cout << "Unable to create file. " << std::endl;
  }
  return;
}
