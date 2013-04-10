#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <Eigen/Dense>
#include <algorithm>

using namespace std;
using namespace Eigen;

typedef SelfAdjointEigenSolver<MatrixXd> MyEigenSolver;

void save_eigenvalues(MyEigenSolver);
void save_eigenvectors(MyEigenSolver);

int main (int argc, char *argv[]) {
  int size, row, col;
  double band_energy[3];
  string line;

  if (argc != 2) {
    cout << "usage: eig file\n       where 'file' is the matrix you want to diagonalize." << endl;
    return 1;
  }   

  // Read the matrix
  ifstream inFile(argv[1]);
  if (inFile) {
    size = count(istreambuf_iterator<char>(inFile),
		 istreambuf_iterator<char>(), '\n');
    inFile.seekg (0, ios::beg);
  } 
  else {
    cout << "I couldn't open the file: " << argv[1] << endl;
    return 1;
  }

  cout << "There are " << size << " lines so I will assume it's a " << size << "x" << size << " matrix." << endl;
  
  MatrixXd m(size,size); 

  for (row = 0; row < size; row++) {
    getline(inFile, line);
    stringstream ss(line);
    for (col = 0; col < size; col ++) {
      float f;
      ss >> f;
      m(row, col) = f;
    }
  }
  
  cout << "I will try to calculate the eigenvalues and eigenvectors now." << endl;
  cout << "This could take some time... ";
  MyEigenSolver eigensolver(m);
  cout << "Done." << endl;

  cout << "Saving the eigenvalues at \"eigenvalues.txt\"... ";
  save_eigenvalues(eigensolver);
  cout << "Done." << endl;

  cout << "Saving the eigenvectors at \"eigenvectors.txt\"... ";
  save_eigenvectors(eigensolver);
  cout << "Done." << endl;

  return 0;
}


void save_eigenvalues(MyEigenSolver eigensolver) {
    ofstream eigvfile;
    eigvfile.open("eigenvalues.txt", ios::out);
    if (eigvfile.is_open()) {
      eigvfile << eigensolver.eigenvalues();
      eigvfile << endl;
      eigvfile.close();
    }
    else {
      cout << "Unable to create file" << endl;
    }
    return;
}


void save_eigenvectors(MyEigenSolver eigensolver) {
    ofstream eigvfile;
    eigvfile.open("eigenvectors.txt", ios::out);
    if (eigvfile.is_open()) {
      eigvfile << eigensolver.eigenvectors();
      eigvfile << endl;
      eigvfile.close();
    }
    else {
      cout << "Unable to create file" << endl;
    }
    return;
}
