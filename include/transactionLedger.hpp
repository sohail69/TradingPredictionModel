#ifndef TRANSACTIONLEDGER_HPP
#define TRANSACTIONLEDGER_HPP

#include <map>
#include <vector>
#include <string>

using namespace std;

typedef struct{
  string            stockName;
  bool              BuyOrSell; 
  unsigned long int BuySellPrice;
  unsigned long int stockQuantity;
  unsigned long int Tax;
  double   TransactionTime;
} transaction;

class DummyIOSocket{
  public:
    DummyIOSocket(){};
    void OutputTransactionLedger(vector<transaction> Ledger){};
    ~DummyIOSocket(){};
};

template<typename IOSocket> //output IOsocket
class TransactionLedger{
  private:
    //All monetary values are given in
    //GBP (pennies) or minimal partial stocks
    map<string,unsigned long int> totalStockCost;
    map<string,unsigned long int> totalStockQuantity;
    map<string,unsigned long int> AverageBuyPrice;
    vector<transaction>           BuySaleLedger;
    unsigned long int             liquidAssets;
    IOSocket                     *outputSocket;
    int LedgerResetLength=100; //n-transactions before writing

  public:
    //Constructs the ledger class 
    TransactionLedger(string stocksConfigFname);

    //Sends current ledger data to file
    void addTransaction(transaction newTransaction);

    //Sends current ledger data to file
    void ResetLedgerToFile();

    //deletes the ledger class
    ~TransactionLedger();
};

template<typename IOSocket>
TransactionLedger<IOSocket>::TransactionLedger(string stocksConfigFname){
  outputSocket = new IOSocket;
};

template<typename IOSocket>
TransactionLedger<IOSocket>::~TransactionLedger(){
  ResetLedgerToFile();
};

template<typename IOSocket>
void TransactionLedger<IOSocket>::ResetLedgerToFile(){
  outputSocket->OutputTransactionLedger(BuySaleLedger);
  BuySaleLedger.clear();
};


template<typename IOSocket>
void TransactionLedger<IOSocket>::addTransaction(transaction newTransaction){
  BuySaleLedger.push_back(newTransaction);
  string sName(newTransaction.stockName);

  if(newTransaction.BuyOrSell == 0){ //Buy
    totalStockCost[sName]     = totalStockCost[sName] + (newTransaction.BuySellPrice);
    totalStockQuantity[sName] = totalStockQuantity[sName] + (newTransaction.stockQuantity);
    liquidAssets              = liquidAssets - (newTransaction.BuySellPrice);
  }

  if(newTransaction.BuyOrSell == 1){ //Sell
    totalStockCost[sName]     = totalStockCost[sName] - AverageBuyPrice[sName]*(newTransaction.stockQuantity);
    totalStockQuantity[sName] = totalStockQuantity[sName] - (newTransaction.stockQuantity);
    liquidAssets              = liquidAssets + (newTransaction.BuySellPrice);
  }
  AverageBuyPrice[sName] = (totalStockCost[sName]/totalStockQuantity[sName]) + 1;
  if(BuySaleLedger.size()==LedgerResetLength) ResetLedgerToFile();
};

#endif





















