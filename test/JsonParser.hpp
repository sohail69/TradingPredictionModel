#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP 

#include <string>
#include <vector>
#include <array>
#include <tuple>
using namespace std;


typedef tuple<string,string,double,double,time_t> Order;

//
// JSon files store data as Key-data pairs
// Each Key has a unique data entry, entries
// are limited to strings, numbers, objects
// and boolean arrays
//


// Returns a value associated
// with a particular JSon
// delimiter
//
int stringParser(string char1){
  const vector<string> separators({" ","{","}","[","]",":","\""});
  int sType=0; //Plain Data
  for(int I=0; I<separators.size(); I++){
    if(char1 == separators[I]) sType = I+1;
    if(char1 == separators[I]) break;
  }
  return sType;
};


// Calculates the number of
// delimiters for some input
// data JSon string
//
void dataDelimCounter(const string InData
                    , array<int,8> *DelimCounts
                    , array<int,2> *DeltaDelimCounts){

  for(int I=0; I<InData.length(); I++){
    string char1 = InData.substr(I,1);
    int K = stringParser(char1);
    (*DelimCounts)[K] =  (*DelimCounts)[K] + 1;
  }
  //Objects (0-Objects) (1-Arrays)
  (*DeltaDelimCounts)[0] += (*DelimCounts)[3] - (*DelimCounts)[2]; //dN-Objects
  (*DeltaDelimCounts)[1] += (*DelimCounts)[5] - (*DelimCounts)[4]; //dN-Arrays
};


// Calculates the maximal object
// depth of a given JSon data string 
// for arrays and objects 
//
void dataObjDepth(const string InData
                , array<int,2> *MaxObjDepth){
  int arrDepth = (*MaxObjDepth)[0];
  int objDepth = (*MaxObjDepth)[1];
  for(int I=0; I<InData.length(); I++){
    string char1 = InData.substr(I,1);
    int K = stringParser(char1);
    if(K == 2) objDepth = objDepth + 1;
    if(K == 3) objDepth = objDepth - 1;
    if(K == 4) arrDepth = arrDepth + 1;
    if(K == 5) arrDepth = arrDepth - 1;
    (*MaxObjDepth)[0] = max(objDepth,(*MaxObjDepth)[0]);
    (*MaxObjDepth)[1] = max(arrDepth,(*MaxObjDepth)[1]);
  }
};


// Puts the data entries from a 
// a JSon input string into a
// vector to be managed or
// analysed later
//


#endif
















