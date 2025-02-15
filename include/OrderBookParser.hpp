#ifndef ORDERBOOKPARSER_HPP
#define ORDERBOOKPARSER_HPP 

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <array>

//
//
//  Orderbook datatype
//
//
typedef std::pair<time_t,std::array<int,2>> OrderBookEntry;

//
//  JSon datatype reader
//
//  Reads in a JSon file with a fixed format
//  configurable on construction and updatable
//  however the output datatype is fixed on
//  compilation
//
template<typename T, unsigned nFields>
class JsonSimpleStructuredDataReader{
  private:
    int SampleObjDepth;
    vector<string> ObjNames;
    //time_t;

    //Reads in a string and checks the type
    int stringParser(std::string);

    //Calculates the depth of an object (depth of nesting)
    int CalcSampleObjectDepth();

    //Calculates the depth of an object (depth of nesting)
    bool keyNotFoundErr(size_t objKey, std::string FieldName);

  public:
    //Reads in a unit piece of data with a fixed form
    JsonSimpleStructuredDataReader(std::string& SampleDataStruct);

    //Takes a data string from a stream and produces a output
    void ReadData(std::string InData, std::vector<T> OutData);
};


template<typename T, unsigned nFields>
JsonSimpleStructuredDataReader<T,nFields>::JsonSimpleStructuredDataReader(std::string& SampleDataStruct){


};


template<typename T, unsigned nFields>
int JsonSimpleStructuredDataReader<T,nFields>::stringParser(std::string char1){
  int sType=0;               //Plain Data
  if(" "  == char1) sType=1; //Whitespace
  if("{"  == char1) sType=2; //Object Opening
  if(":"  == char1) sType=3; //Object Definition
  if("["  == char1) sType=4; //Array Opening
  if("\"" == char1) sType=5; //String opening/closing
  return sType;
};


template<typename T, unsigned nFields>
void JsonSimpleStructuredDataReader<T,nFields>::ReadData(std::string InData, std::vector<T> OutData){
  size_t pos=0;
  size_t len=InData.length();

    std::vector<std::string> FieldNames;
    std::vector<std::string> FieldSeparators;
    std::vector<size_t>      nSeparators;
    std::vector<std::array<double, nFields>> Data;

    while(pos < len){
      // Find the start of the next templated object
      size_t keyStart = InData.find('"', pos);
      if (keyStart == std::string::npos) break; // No more keys found

      // Find the end of the key
      size_t keyEnd = InData.find('"', keyStart + 1);
      if (keyEnd == std::string::npos) break; // Malformed JSON

      // Find the start of the object associated with the currency pair
      size_t objStart = InData.find('{', keyEnd);
      if (objStart == std::string::npos) break; // Malformed JSON

      // Find the end of the object
      size_t objEnd = InData.find('}', objStart);
      if (objEnd == std::string::npos) break; // Malformed JSON
      std::string objContent = InData.substr(objStart + 1, objEnd - objStart - 1);


      //Parse the data from a fixed format type
      //given in the constructor and parsed
      //
      for(int J=0; J<FieldNames.size(); J++){
        size_t FieldKey = objContent.find(FieldNames[J]);
        if (FieldKey == std::string::npos) std::cerr << "Warning:"+FieldNames[J]+"field not found." << std::endl;
        if (FieldKey == std::string::npos) continue; //skip malformed data
        size_t FieldSepPos[nSeparators[J]];
        for(int I=0; I<nSeparators[J]; I++){
          FieldSepPos[I] = objContent.find(FieldSeparators[I]);
          if(FieldSepPos[I] != std::string::npos) std::cerr << FieldSeparators[I]+" Malformed." << std::endl;
/*
          std::string bidStr = objContent.substr(colon + 1, comma - colon - 1);
          bidStr.erase(0, bidStr.find_first_not_of(" \t\n\r")); // Trim leading whitespace
          bidStr.erase(bidStr.find_last_not_of(" \t\n\r") + 1); // Trim trailing whitespace
          bid = std::stod(bidStr);
*/
        }
      }
    };


};

template<typename T, unsigned nFields>
bool JsonSimpleStructuredDataReader<T,nFields>::keyNotFoundErr(size_t objKey, std::string FieldName){
  bool keyNotFound=false;
  if (objKey != std::string::npos) keyNotFound = true;
  if (objKey != std::string::npos) std::cerr<<"Warning: "+FieldName+" field not found."<< std::endl;
  return keyNotFound;
};
#endif
