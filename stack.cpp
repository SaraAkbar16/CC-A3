#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <map>
using namespace std;

const string EPSILON = "#";
vector<string> inputTokens;

// Function to load tokens from a string
void loadInputTokens(const string& input) {
    stringstream ss(input);
    string token;
    while (ss >> token) {
        if (token == "x") token = "id"; // replace x with id
        inputTokens.push_back(token);
    }
    inputTokens.push_back("$"); // End marker
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

// Load the parsing table from file into map<nonTerminal, map<terminal, production>>
map<string, map<string, string>> loadParsingTable(const string& filename) {
    map<string, map<string, string>> table;
    ifstream file(filename);
    string line, currentNonTerminal;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // If line has no '->', it's a non-terminal heading
        if (line.find("->") == string::npos) {
            currentNonTerminal = line;
        } else {
            // Parse terminal -> NonTerminal -> production
            stringstream ss(line);
            string terminal, arrow1, lhs, arrow2, production;
            ss >> terminal >> arrow1 >> lhs >> arrow2;
            getline(ss, production); // rest of line is the production

            production = trim(production);
            table[lhs][terminal] = production;
        }
    }

    return table;
}

void displayStack(const stack<string>& parseStack) {
    // Display the current stack from top to bottom
    stack<string> tempStack = parseStack;
    cout << "Stack: ";
    while (!tempStack.empty()) {
        cout << tempStack.top() << " ";
        tempStack.pop();
    }
    cout << endl;
}
void parseStringWithLL1FromFileTable(const string& input, const map<string, map<string, string>>& table) {
    stack<string> parseStack;
    parseStack.push("$");
    parseStack.push("S");  // Assuming S is the start symbol

    string inputStr = input + " $";
    stringstream ss(inputStr);
    vector<string> tokens;
    string tok;

    while (ss >> tok) {
        tokens.push_back(tok);
    }

    size_t index = 0;

    // Open a log file to write the output
    ofstream logFile("ll1_log.txt");
    if (!logFile.is_open()) {
        cout << "Error opening log file." << endl;
        return;
    }

    // Start parsing
    while (!parseStack.empty()) {
        string top = parseStack.top();
        string currentToken = (index < tokens.size()) ? tokens[index] : "$";

        // Log the current state
        logFile << "Stack top: " << top << ", Current Token: " << currentToken << endl;
        cout << "Stack top: " << top << ", Current Token: " << currentToken << endl;

        // Display the current stack
        displayStack(parseStack);

        // Check if stack has only one element and it is "$"
        if (parseStack.size() == 1 && top == "$" && currentToken == "$") {
            logFile << "String parsed successfully.\n";
            cout << "String parsed successfully.\n";
            logFile.close();
            return;
        }

        // Terminal or $ (Handle matching terminal and stack)
        if (top == currentToken) {
            parseStack.pop();
            ++index;
        } else if (top == "$") {
            if (currentToken == "$") {
                logFile << "String parsed successfully.\n";
                cout << "String parsed successfully.\n";
                logFile.close();
                return;
            } else {
                logFile << "Error: unexpected input after end of parsing.\n";
                cout << "Error: unexpected input after end of parsing.\n";
                logFile.close();
                return;
            }
        } else {
            // Non-terminal logic (Look for matching production in the table)
            if (table.find(top) != table.end()) {
                const auto& row = table.at(top);
                if (row.find(currentToken) != row.end()) {
                    string production = row.at(currentToken);
                    logFile << "Apply rule: " << top << " -> " << production << endl;
                    cout << "Apply rule: " << top << " -> " << production << endl;
                    parseStack.pop();

                    if (production != EPSILON) {
                        // Push in reverse order
                        stringstream prodStream(production);
                        vector<string> symbols;
                        string sym;
                        while (prodStream >> sym) {
                            symbols.push_back(sym);
                        }
                        for (int i = symbols.size() - 1; i >= 0; --i) {
                            parseStack.push(symbols[i]);
                        }
                    }
                } else {
                    logFile << "Error: No rule for [" << top << "] with input [" << currentToken << "]\n";
                    cout << "Error: No rule for [" << top << "] with input [" << currentToken << "]\n";
                    logFile.close();
                    return;
                }
            } else {
                logFile << "Error: Unknown non-terminal [" << top << "]\n";
                cout << "Error: Unknown non-terminal [" << top << "]\n";
                logFile.close();
                return;
            }
        }
    }

    if (index == tokens.size()) {
        logFile << "String parsed successfully.\n";
        cout << "String parsed successfully.\n";
    } else {
        logFile << "Error: Input not fully consumed.\n";
        cout << "Error: Input not fully consumed.\n";
    }

    logFile.close();
}
