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

template<typename IOSocket> //output IOsocket
class TransactionLedger{
  private:
    //All monetary values are given in
    //GBP (pennies) or minimal partial stocks
    vector<transaction>          BuySaleLedger;
    vector<unsigned long int>    totalStockCost;
    vector<unsigned long int>    totalStockQuantity;
    vector<unsigned long int>    AverageBuyPrice;
    unsigned long int            liquidAssets;
    IOSocket                     outputSocket;

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
};

template<typename IOSocket>
TransactionLedger<IOSocket>::~TransactionLedger(){
  ResetLedgerToFile();
};
