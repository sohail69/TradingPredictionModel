#include <cmath>
#include <iomanip>
#include <vector>
#include <array>

#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

//Json File Parsing
#include "../include/JsonParser.hpp"

// Compile with:
// g++ -std=c++17 -O2 -o main test6.cpp
//
using namespace std;

int main(){
  string strData;
  array<int,8>   DelimCounts;
  array<int,2>   DeltaDelimCounts, MaxObjDepth;
  vector<string> FieldNames;
  string fName="../log.Data";
  ifstream InFile(fName);  //The output 

  if(InFile){
    ostringstream ss;
    ss << InFile.rdbuf(); // reading data
    strData = ss.str();
    dataDelimCounter(strData, &DelimCounts, &DeltaDelimCounts);
    dataObjDepth(strData, &MaxObjDepth);
  }

  //Object delimiter counts
  for(int J=0; J<8; J++) cout << setw(8) << DelimCounts[J];
  cout << endl;

  cout << setw(8) << DeltaDelimCounts[0]
       << setw(8) << DeltaDelimCounts[1] << endl;



  cout << setw(8) << MaxObjDepth[0]
       << setw(8) << MaxObjDepth[1] << endl;



  InFile.close();
  return 0;
};
