#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <utility>
#include <iterator>
#include <cfloat> 
#include <fstream>
using namespace std;

#include <thread>
#include <chrono>
#include <iomanip>   // for setw

// 🎨 COLOR AND STYLE CODES
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

// ✨ CLEAR SCREEN FUNCTION
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}



// ✨ DIVIDER FUNCTION
void printDivider(string title = "") {
    cout << BOLD << MAGENTA;
    cout << "\n============================================================\n";
    if (!title.empty()) cout << "   " << title << "\n";
    cout << "============================================================\n" << RESET;
}

// ✨ HEADER FUNCTION
void printHeader() {
    cout << BOLD << BLUE;
    cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    cout << "║             💰 CASH FLOW MINIMIZER SYSTEM 💰             ║\n";
    cout << "║            Developed as part of DSA Project              ║\n";
    cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    cout << RESET;
}



class bank {
public:
    string name;
    int netAmount;

    // Instead of just a set of types, store details for each mode
    struct ModeInfo {
        int priority;       // Higher number = higher priority
        double costPercent; // Transaction cost (in %)
        int truncation;     // Max amount allowed per transaction using this mode
    };

    unordered_map<string, ModeInfo> modes;  // mode name → details
    set<string> types;                      // Still keep for intersection
};

// 🌍 Predefined World Bank Modes (standardized rules)
unordered_map<string, bank::ModeInfo> defaultWorldBankModes = {
    {"upi",   {5, 1.2, 10000}},   // small fast payments
    {"imps",  {4, 0.8, 5000}},    // micro transactions
    {"neft",  {3, 0.5, 50000}},   // medium range
    {"rtgs",  {2, 0.3, 200000}},  // large-value transactions
    {"swift", {1, 0.2, 1000000}}  // international/very large
};




int getMinIndex(bank listOfNetAmounts[],int numBanks){
    int min=INT_MAX, minIndex=-1;
    for(int i=0;i<numBanks;i++){
        if(listOfNetAmounts[i].netAmount == 0) continue;
        
        if(listOfNetAmounts[i].netAmount < min){
            minIndex = i;
            min = listOfNetAmounts[i].netAmount;
        }
    }
    return minIndex;
}

int getSimpleMaxIndex(bank listOfNetAmounts[],int numBanks){
    int max=INT_MIN, maxIndex=-1;
    for(int i=0;i<numBanks;i++){
        if(listOfNetAmounts[i].netAmount == 0) continue;
        
        if(listOfNetAmounts[i].netAmount > max){
            maxIndex = i;
            max = listOfNetAmounts[i].netAmount;
        }
    }
    return maxIndex;
}

pair<int, string> getMaxIndex(bank listOfNetAmounts[], int numBanks, int minIndex, bank input[], int maxNumTypes) {
    int max = INT_MIN;
    int maxIndex = -1;
    string bestMode;

    for (int i = 0; i < numBanks; i++) {
        if (listOfNetAmounts[i].netAmount <= 0) continue;  // skip debtors

        // Find all common modes between minIndex and i
        vector<string> commonModes;
        set_intersection(
            listOfNetAmounts[minIndex].types.begin(), listOfNetAmounts[minIndex].types.end(),
            listOfNetAmounts[i].types.begin(), listOfNetAmounts[i].types.end(),
            back_inserter(commonModes)
        );

        if (commonModes.empty()) continue;

        // Select the best mode based on priority and cost
        string selectedMode;
        int bestPriority = -1;
        double lowestCost = DBL_MAX;

        for (const string& mode : commonModes) {
            int p1 = input[minIndex].modes[mode].priority;
            int p2 = input[i].modes[mode].priority;
            double c1 = input[minIndex].modes[mode].costPercent;
            double c2 = input[i].modes[mode].costPercent;

            // Average priority and cost for fairness
            int avgPriority = (p1 + p2) / 2;
            double avgCost = (c1 + c2) / 2;

            if (avgPriority > bestPriority ||
               (avgPriority == bestPriority && avgCost < lowestCost)) {
                selectedMode = mode;
                bestPriority = avgPriority;
                lowestCost = avgCost;
            }
        }

        // Choose creditor with highest credit and best mode
        if (!selectedMode.empty() && listOfNetAmounts[i].netAmount > max) {
            max = listOfNetAmounts[i].netAmount;
            maxIndex = i;
            bestMode = selectedMode;    
        }
    }

    return make_pair(maxIndex, bestMode);
}


void printAns(vector<vector<pair<int,string> > > ansGraph, int numBanks,bank input[]){
    
    cout << GREEN << "\nThe transactions for minimum cash flow are as follows:\n\n" << RESET;

    // 🧮 Summary tracking variables
    int totalTransactions = 0;
    double totalTransferred = 0.0;
    double totalCostPaid = 0.0;


for (int i = 0; i < numBanks; i++) {
    for (int j = 0; j < numBanks; j++) {

        if (i == j) continue;

        // Case 1: both have mutual debts
        if (ansGraph[i][j].first != 0 && ansGraph[j][i].first != 0) {

            if (ansGraph[i][j].first == ansGraph[j][i].first) {
                ansGraph[i][j].first = 0;
                ansGraph[j][i].first = 0;
            } 
            else if (ansGraph[i][j].first > ansGraph[j][i].first) {
                ansGraph[i][j].first -= ansGraph[j][i].first;
                ansGraph[j][i].first = 0;

                cout << YELLOW << left << setw(15) << input[i].name << RESET
                     << " pays " << RED << "Rs " << setw(8) << ansGraph[i][j].first << RESET
                     << " to " << CYAN << left << setw(15) << input[j].name << RESET
                     << " via " << MAGENTA << ansGraph[i][j].second << RESET << endl;

                // ✅ Update summary statistics after each transaction
                totalTransactions++;
                totalTransferred += ansGraph[j][i].first;
                // Estimate transaction cost using mode’s average cost percent
                string mode = ansGraph[j][i].second;
                if (!mode.empty()) {

                    double avgCost = 0.0;
                     if (input[i].modes.find(mode) != input[i].modes.end() && 
                     input[j].modes.find(mode) != input[j].modes.end()) {
                        avgCost = (input[i].modes[mode].costPercent + input[j].modes[mode].costPercent) / 2.0;
                    }
                    totalCostPaid += ansGraph[j][i].first * (avgCost / 100.0);
                }
     
            } 
            else {
                ansGraph[j][i].first -= ansGraph[i][j].first;
                ansGraph[i][j].first = 0;

                cout << YELLOW << left << setw(15) << input[j].name << RESET
                     << " pays " << RED << "Rs " << setw(8) << ansGraph[j][i].first << RESET
                     << " to " << CYAN << left << setw(15) << input[i].name << RESET
                     << " via " << MAGENTA << ansGraph[j][i].second << RESET << endl;
            }
        }

        // Case 2: only one direction debt
        else if (ansGraph[i][j].first != 0) {
            cout << YELLOW << left << setw(15) << input[i].name << RESET
                 << " pays " << RED << "Rs " << setw(8) << ansGraph[i][j].first << RESET
                 << " to " << CYAN << left << setw(15) << input[j].name << RESET
                 << " via " << MAGENTA << ansGraph[i][j].second << RESET << endl;
        // ✅ Update summary statistics after each transaction
totalTransactions++;
totalTransferred += ansGraph[j][i].first;

// Estimate transaction cost using mode’s average cost percent
string mode = ansGraph[j][i].second;
if (!mode.empty()) {
    double avgCost = 0.0;
    if (input[i].modes.find(mode) != input[i].modes.end() && 
        input[j].modes.find(mode) != input[j].modes.end()) {
        avgCost = (input[i].modes[mode].costPercent + input[j].modes[mode].costPercent) / 2.0;
    }
    totalCostPaid += ansGraph[j][i].first * (avgCost / 100.0);
}
}

        else if (ansGraph[j][i].first != 0) {
            cout << YELLOW << left << setw(15) << input[j].name << RESET
                 << " pays " << RED << "Rs " << setw(8) << ansGraph[j][i].first << RESET
                 << " to " << CYAN << left << setw(15) << input[i].name << RESET
                 << " via " << MAGENTA << ansGraph[j][i].second << RESET << endl;
        
        // ✅ Update summary statistics after each transaction
totalTransactions++;
totalTransferred += ansGraph[j][i].first;

// Estimate transaction cost using mode’s average cost percent
string mode = ansGraph[j][i].second;
if (!mode.empty()) {
    double avgCost = 0.0;
    if (input[i].modes.find(mode) != input[i].modes.end() && 
        input[j].modes.find(mode) != input[j].modes.end()) {
        avgCost = (input[i].modes[mode].costPercent + input[j].modes[mode].costPercent) / 2.0;
    }
    totalCostPaid += ansGraph[j][i].first * (avgCost / 100.0);
}
        }

        ansGraph[i][j].first = 0;
        ansGraph[j][i].first = 0;
    }
}

// 🎯 Final Summary Section
printDivider("TRANSACTION SUMMARY");
cout << fixed << setprecision(2);
cout << GREEN << "Total Transactions: " << RESET << totalTransactions << "\n";
cout << CYAN  << "Total Amount Transferred: Rs " << RESET << totalTransferred << "\n";
cout << YELLOW << "Total Transaction Cost Paid: Rs " << RESET << totalCostPaid << "\n";
cout << MAGENTA << "Average Transaction Cost: " << RESET 
     << (totalTransactions > 0 ? (totalCostPaid / totalTransferred * 100.0) : 0.0)
     << "%\n";

cout << "\n";

}

void minimizeCashFlow(int numBanks,bank input[],unordered_map<string,int>& indexOf,int numTransactions,vector<vector<int > >& graph,int maxNumTypes){
    
    //Find net amount of each bank has
    bank listOfNetAmounts[numBanks];
    
    for(int b=0;b<numBanks;b++){
        listOfNetAmounts[b].name = input[b].name;
        listOfNetAmounts[b].types = input[b].types;
        
        int amount = 0;
        //incoming edges
        //column travers
        for(int i=0;i<numBanks;i++){
            amount += (graph[i][b]);
        }
        
        //outgoing edges
        //row traverse
        for(int j=0;j<numBanks;j++){
            amount += ((-1) * graph[b][j]);
        }
        
        listOfNetAmounts[b].netAmount = amount;
    }
    
    vector<vector<pair<int,string > > > ansGraph(numBanks,vector<pair<int,string > >(numBanks, make_pair(0, "")));//adjacency matrix
    
    
    //find min and max net amount
    int numZeroNetAmounts=0;
    
    for(int i=0;i<numBanks;i++){
        if(listOfNetAmounts[i].netAmount == 0) numZeroNetAmounts++;
    }
    while(numZeroNetAmounts!=numBanks){
        
        int minIndex=getMinIndex(listOfNetAmounts, numBanks);
        pair<int,string> maxAns = getMaxIndex(listOfNetAmounts, numBanks, minIndex,input,maxNumTypes);
        
        int maxIndex = maxAns.first;
        
        if (maxIndex == -1) {
    int worldBank = 0;
    int simpleMaxIndex = getSimpleMaxIndex(listOfNetAmounts, numBanks);
    int amount = abs(listOfNetAmounts[minIndex].netAmount);

    // Pick default modes for each leg of transaction
    string debtorMode = *(input[minIndex].types.begin());
    string creditorMode = *(input[simpleMaxIndex].types.begin());

    // ✅ Step 1: Apply truncation and cost for Debtor → World Bank
    int truncLimit1 = INT_MAX;
    double costPercent1 = 0.0;
    if (input[minIndex].modes.find(debtorMode) != input[minIndex].modes.end()) {
        truncLimit1 = input[minIndex].modes[debtorMode].truncation;
        costPercent1 = input[minIndex].modes[debtorMode].costPercent;
    }
    int transactionAmount1 = min(amount, truncLimit1);
    double costAmount1 = transactionAmount1 * (costPercent1 / 100.0);

    (ansGraph[minIndex][worldBank].first) += transactionAmount1 + costAmount1;
    (ansGraph[minIndex][worldBank].second) = debtorMode;

    // ✅ Step 2: Apply truncation and cost for World Bank → Creditor
    int truncLimit2 = INT_MAX;
    double costPercent2 = 0.0;
    if (input[simpleMaxIndex].modes.find(creditorMode) != input[simpleMaxIndex].modes.end()) {
        truncLimit2 = input[simpleMaxIndex].modes[creditorMode].truncation;
        costPercent2 = input[simpleMaxIndex].modes[creditorMode].costPercent;
    }
    int transactionAmount2 = min(transactionAmount1, truncLimit2);
    double costAmount2 = transactionAmount2 * (costPercent2 / 100.0);

    (ansGraph[worldBank][simpleMaxIndex].first) += transactionAmount2 + costAmount2;
    (ansGraph[worldBank][simpleMaxIndex].second) = creditorMode;

    // ✅ Step 3: Update balances
    listOfNetAmounts[simpleMaxIndex].netAmount += listOfNetAmounts[minIndex].netAmount;
    listOfNetAmounts[minIndex].netAmount = 0;

    if (listOfNetAmounts[minIndex].netAmount == 0) numZeroNetAmounts++;
    if (listOfNetAmounts[simpleMaxIndex].netAmount == 0) numZeroNetAmounts++;
}

     else {
    // ✅ Step 4: Automatically choose the best payment mode based on transaction size
    int maxIndex = maxAns.first;

    int amountToTransfer = abs(listOfNetAmounts[minIndex].netAmount);
    string modeUsed;

    if (amountToTransfer <= 5000)
        modeUsed = "imps";
    else if (amountToTransfer <= 10000)
        modeUsed = "upi";
    else if (amountToTransfer <= 50000)
        modeUsed = "neft";
    else if (amountToTransfer <= 200000)
        modeUsed = "rtgs";
    else
        modeUsed = "swift";

    if (input[minIndex].modes.find(modeUsed) == input[minIndex].modes.end() ||
        input[maxIndex].modes.find(modeUsed) == input[maxIndex].modes.end()) {

        for (auto &m : input[minIndex].types) {
            if (input[maxIndex].types.find(m) != input[maxIndex].types.end()) {
                modeUsed = m;
                break;
            }
        }
    }

    // 1️⃣ Get truncation limit
    int truncLimit = INT_MAX;
    if (input[minIndex].modes.find(modeUsed) != input[minIndex].modes.end() &&
        input[maxIndex].modes.find(modeUsed) != input[maxIndex].modes.end()) {

        truncLimit = min(input[minIndex].modes[modeUsed].truncation,
                         input[maxIndex].modes[modeUsed].truncation);
    }

    // 2️⃣ Determine transferable amount
    int possibleAmount = min(abs(listOfNetAmounts[minIndex].netAmount),
                             listOfNetAmounts[maxIndex].netAmount);
    int transactionAmount = min(possibleAmount, truncLimit);

    // 3️⃣ Apply transaction cost
    double costPercent = (input[minIndex].modes[modeUsed].costPercent +
                          input[maxIndex].modes[modeUsed].costPercent) / 2.0;
    double costAmount = transactionAmount * (costPercent / 100.0);
    int totalTransfer = transactionAmount + static_cast<int>(costAmount);

    // 4️⃣ Record in result graph
    ansGraph[minIndex][maxIndex].first += totalTransfer;
    ansGraph[minIndex][maxIndex].second = modeUsed;

    // 5️⃣ Update balances
    listOfNetAmounts[minIndex].netAmount += transactionAmount;
    listOfNetAmounts[maxIndex].netAmount -= transactionAmount;

    if (listOfNetAmounts[minIndex].netAmount == 0) numZeroNetAmounts++;
    if (listOfNetAmounts[maxIndex].netAmount == 0) numZeroNetAmounts++;
   }


        
    }
    
    clearScreen();
    printDivider("RESULTS - MINIMUM CASH FLOW TRANSACTIONS");
    
    cout << CYAN << "\nProcessing transactions, please wait...\n" << RESET;
    


    printAns(ansGraph,numBanks,input);

    cout << BOLD << BLUE
     << "\n============================================================\n"
     << "   ✅  All transactions settled successfully!\n"
     << "   Thank you for using the Cash Flow Minimizer System.\n"
     << "============================================================\n"
     << RESET;

    // cout<<"HI\n";
}

//correct
int main()
{
    clearScreen();             // clears the terminal
    printHeader();             // shows the blue ASCII banner
    cout << GREEN 
         << "This system minimizes the number of transactions among multiple banks\n"
         << "that use different payment modes. The World Bank acts as an intermediary\n"
         << "between banks that have no common mode of payment.\n\n" 
         << RESET;

    printDivider("BANK SETUP");   // adds magenta divider with a title
    cout << "Enter the number of banks participating in the transactions: ";

    int numBanks;
    cin >> numBanks;
    
    printDivider("BANK DETAILS INPUT");

    bank input[numBanks];
    unordered_map<string,int> indexOf;//stores index of a bank
    
    cout<<"Enter the details of the banks and transactions as stated:\n";
    cout<<"Bank name ,number of payment modes it uses.\n";
    cout<<"Bank name and payment modes should not contain spaces\n";
    
    int maxNumTypes;
    for (int i = 0; i < numBanks; i++) {
    if (i == 0) {
        // 🌍 Predefined World Bank modes
        input[i].name = "WorldBank";
        indexOf[input[i].name] = i;

        cout << CYAN << "\nWorld Bank payment modes are pre-configured:\n" << RESET;
        for (auto &m : defaultWorldBankModes) {
            input[i].modes[m.first] = m.second;
            input[i].types.insert(m.first);
            cout << "  • " << m.first
                 << " | Priority: " << m.second.priority
                 << " | Truncation: " << m.second.truncation
                 << " | Cost: " << m.second.costPercent << "%\n";
        }
        maxNumTypes = defaultWorldBankModes.size();
        continue; // ✅ Skip manual entry for World Bank
    }

    // For other banks
    cout << "\nEnter bank name: ";
    cin >> input[i].name;
    indexOf[input[i].name] = i;

    cout << "Enter number of payment modes used by " << input[i].name << ": ";
    int numTypes;
    cin >> numTypes;

    cout << "Enter the mode names (e.g. upi, neft, rtgs...)\n";

    for (int t = 0; t < numTypes; ++t) {
        string mode;
        cin >> mode;
        input[i].types.insert(mode);

        // ✅ Use World Bank defaults if available
        if (defaultWorldBankModes.find(mode) != defaultWorldBankModes.end()) {
            input[i].modes[mode] = defaultWorldBankModes[mode];
        } else {
            // If it’s a custom mode, ask for values
            cout << "Enter truncation limit and cost percent for " << mode << ":\n";
            int trunc;
            double cost;
            cin >> trunc >> cost;
            input[i].modes[mode] = {1, cost, trunc};
        }
    }

    // Confirmation print
    cout << GREEN << "Configured payment modes for " << input[i].name << ":\n" << RESET;
    for (auto &m : input[i].modes) {
        cout << "  • " << m.first
             << " | Truncation: " << m.second.truncation
             << " | Cost: " << m.second.costPercent
             << " | Priority: " << m.second.priority << "\n";
    }
}


    cout << "\n==================== BANK MODE DETAILS ====================\n";
    for (int i = 0; i < numBanks; i++) {
    cout << "\nBank: " << input[i].name << "\n";
    cout << "Supported Payment Modes:\n";
    for (auto &m : input[i].modes) {
        cout << "  • " << m.first
             << " | Priority: " << m.second.priority
             << " | Truncation Limit: " << m.second.truncation
             << " | Transaction Cost: " << m.second.costPercent << "%\n";
    }
    }
    cout << "===========================================================\n\n";

    
    printDivider("TRANSACTION DETAILS");
    cout<<"Enter number of transactions.\n";
    int numTransactions;
    cin>>numTransactions;
    
    vector<vector<int > > graph(numBanks,vector<int>(numBanks,0));//adjacency matrix
    
    cout<<"Enter the details of each transaction as stated:";
    cout<<"Debtor Bank , creditor Bank and amount\n";
    cout<<"The transactions can be in any order\n";
    for(int i=0;i<numTransactions;i++){
        cout<<(i)<<" th transaction : ";
        string s1,s2;
        int amount;
        cin >> s1>>s2>>amount;
        
        graph[indexOf[s1]][indexOf[s2]] = amount;
    }
     
    //settle
    minimizeCashFlow(numBanks,input,indexOf,numTransactions,graph,maxNumTypes);
    return 0; 
}