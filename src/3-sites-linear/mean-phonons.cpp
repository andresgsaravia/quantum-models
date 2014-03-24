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

int state_label (int e1, int e2, int ir, int ram, int n_ir)
{
  return e1 + (3 * (e2 -1)) + (9 * ir) + (9 * ram * (n_ir + 1)) - 1;
}


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

  VectorXd mean_ir = VectorXd::Zero(size);
  VectorXd mean_ram = VectorXd::Zero(size);
  VectorXd stdd_ir = VectorXd::Zero(size);
  VectorXd stdd_ram = VectorXd::Zero(size);

  cout << "Calculating mean phonons and standard deviations. ";
  // Calculate the mean phonons
  for (n = 0; n < size; n++) {
    float sqr_ir = 0;
    float sqr_ram = 0;
    for (e1 = 1; e1 <= 3; e1++) {
      for (e2 = 1; e2 <= 3; e2++) {
  	for (ir = 0; ir <= ir_phonons; ir ++) {
  	  for (ram = 0; ram <= raman_phonons; ram++) {
  	    mean_ir(n) += ir * pow(eigenvectors(state_label(e1, e2, ir, ram, ir_phonons), n) , 2);
  	    mean_ram(n) += ram * pow(eigenvectors(state_label(e1, e2, ir, ram, ir_phonons), n) , 2);
  	    sqr_ir += pow((float) ir, 2) * pow(eigenvectors(state_label(e1, e2, ir, ram, ir_phonons), n), 2);
  	    sqr_ram += pow((float) ram, 2) * pow(eigenvectors(state_label(e1, e2, ir, ram, ir_phonons), n), 2);
	  }
  	}
      }
    }
    stdd_ir(n) = sqrt(sqr_ir - pow(mean_ir(n), 2));
    stdd_ram(n) = sqrt(sqr_ram - pow(mean_ram(n), 2));
  }
  cout << "Done. " << endl;
  
  cout << "Saving mean infrared phonons at \"mean_ir.txt\"... ";
  save_vector("mean_ir.txt", mean_ir);
  cout << "Done. " << endl;

  cout << "Saving mean raman phonons at \"mean_ram.txt\"... ";
  save_vector("mean_ram.txt", mean_ram);
  cout << "Done. " << endl;

  cout << "Saving standard deviation for the mean infrared phonons at \"stdd_ir.txt\"... ";
  save_vector("stdd_ir.txt", stdd_ir);
  cout << "Done. " << endl;

  cout << "Saving standard deviation for the mean raman phonons at \"stdd_ram.txt\"... ";
  save_vector("stdd_ram.txt", stdd_ram);
  cout << "Done. " << endl;
	    
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
