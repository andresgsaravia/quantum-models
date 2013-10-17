#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

IOFormat LongPrinting(20);

void save_vector(string, VectorXd);

int main (int argc, char *argv[]) {
  int ir_phonons, raman_phonons, size;
  double band_energy[3];
  double nn_hopping, on_site_repulsion, ir_energy, raman_energy, raman_shift;
  double e_ir_coupling, e_ram_coupling;

  int e1, e2, ir, ram, row, col, n;  // just counters

  ifstream inputfile;
  string line, val;
  // Reading calculation parameters
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
    cout << "Input file \"parameters.inp\" not found. I can't proceed any further. " << endl;
    return 1;
  }

  size = 9 * (1 + raman_phonons) * (1 + ir_phonons);
  MatrixXd eigenvectors(size,size);

  // Read the eigenvectors
  inputfile.open("eigenvectors.txt");
  if (inputfile.is_open()) {
    for (row = 0; row < size; row++) {
      getline(inputfile, line);
      stringstream ss(line);
      for (col = 0; col < size; col ++) {
	float f;
	ss >> f;
	eigenvectors(row, col) = f;
      }
    }
    cout << "Eigenvector's matrix has size: " << eigenvectors.rows() << "x" << eigenvectors.cols() << endl;
  }
  else {
    cout << "eigenvectors.txt not found. " << endl;
    return 1;
  }

  VectorXd one_vec = VectorXd::Zero(size);
  stringstream sstm;
  string filename;

  // Calculate the mean phonons
  for (col = 0; col < 20; col++) {
    for (row = 0; row < size; row++) {
      one_vec(row) = eigenvectors(row, col);
    }
    sstm << "v" << col << ".txt";
    filename = sstm.str();
    save_vector(filename, one_vec);
    sstm.clear();
    sstm.str("");
  }
	    
  return 0;
}

void save_vector(string filename, VectorXd vec) {
  ofstream outfile;
  outfile.open(filename.c_str(), ios::out);
  if(outfile.is_open()) {
    outfile << vec.format(LongPrinting);
    outfile << endl;
    outfile.close();
  }
  else {
    cout << "Unable to create file." << endl;
  }
  return;
}
