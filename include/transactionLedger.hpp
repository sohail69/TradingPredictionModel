#include <vector>
#include <string>

using namespace std;

typedef struct{
  string stockName;
  bool BuyOrSell; 
  unsigned long int BuySellPrice;
  unsigned long int stockQuantity;
  unsigned long int Tax;
  double   TransactionTime;
} transaction;

class TransactionLedger{
  private:
    vector<transaction>          BuySaleLedger;
    vector<unsigned long int>    totalStockCost;
    vector<unsigned long int>    totalStockQuantity;
    vector<unsigned long int>    AverageBuyPrice;


    int *StockPriceHistory;
	int LedgerResetLength=100; //n-transactions before writing
    int nStockTypes=20, nHistory=1000;

  public:
    //Constructs the ledger class 
    TransactionLedger(string stocksConfigFname);

    //Sends current ledger data to file
    ResetLedgerToFile();

    //deletes the ledger class
    ~TransactionLedger();
};


TransactionLedger::TransactionLedger(string stocksConfigFname){
  StockPriceHistory = new long int[nStockTypes*nHistory];
}

TransactionLedger::~TransactionLedger(){
  ResetLedgerToFile();
  delete[] StockPriceHistory;
}