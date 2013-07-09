#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void join (uint64_t item) {

  uint64_t token1_A, token2_A, token1_B, token2_B, i = 0;
  string token3_A, token3_B;
  ifstream tableA, tableB;
  
  tableA.open ("table_a.dat", ifstream::in);
  tableB.open ("table_b.dat", ifstream::in);

  if (tableA.bad () || tableB.bad ()) return;

	//! Search item in tableA
  while (!tableA.eof () && (token1_A != item)) {

    tableA >> token1_A >> token2_A >> token3_A;

    while (!tableB.eof () && (token1_B != token1_A)) {
      tableB >> token1_B >> token3_B;
      i++;
    }
  }
  cout << "HERE: " << token3_A << " | " << token3_B << " | " << i<< endl;

	//! Search all the queries in tableB which are same as that query in tableA
}



int main () {
  join (500000);
	return 0;
}

