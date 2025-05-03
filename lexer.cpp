#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;


enum TokenType {
    KEYWORD, IDENTIFIER, INT_LITERAL, CHAR_LITERAL, REL_OP,
    
    ASSIGN, ARITH_OP, PUNCTUATION, ERROR, INT, CHAR, IF, ELSE, WHILE, 
};

string tokenToString(TokenType type) {
    switch (type) {
        case KEYWORD: return "^";
        case INT: return "int";
        case CHAR: return "char";
        case WHILE: return "while";
        case IF: return "if";
        case ELSE: return "else";
        case IDENTIFIER: return "ID";
        case INT_LITERAL: return "NUM";
        case CHAR_LITERAL: return "CL";
        case REL_OP: return "RO";
        case ASSIGN: return "^";
        case ARITH_OP: return "^";
        case PUNCTUATION: return "^";
        case ERROR: return "ERROR";
        
        default: return "UNK";
    }
}


struct Token {
    TokenType type;
    string lexeme;
    int line;
};

unordered_set<string> keywords = {"int", "char", "if", "else", "while"};


void writeTokensToFile(const string &outputFile, const vector<Token> &tokens) {
    ofstream outFile(outputFile);
    if (!outFile) {
        cerr << "Error opening output file!" << endl;
        return;
    }
    int i=1;
    for (const auto &token : tokens) {
        
        if (token.line!=i)
        {
            i++;
            outFile<<endl;
        }
        if (tokenToString(token.type)=="ERROR")
        {
            outFile << "( '" <<token.lexeme   << "',  " << tokenToString(token.type) << "at line  "<<token.line<<")" ;
        }
        else 
        {
            if(token.type==KEYWORD||token.type==REL_OP)
            {
          outFile << "( " <<token.lexeme   << ",  " << tokenToString(token.type) << ")" ;}
          else{
            outFile << "('" <<token.lexeme   << "',  " << tokenToString(token.type) << ")" ;
          }
        }
    
        
    }
    outFile.close();
}

void gettoken(const string &input, const string &output) {
    ifstream file(input);
    if (!file) {
        cerr << "Error opening input file!" << endl;
        return;
    }

    vector<Token> tokens;
    string lexeme = "";
    char ch;
    int line = 1;

    while (file.get(ch)) {
        if (isspace(ch)) {
            if (ch == '\n') line++; 
            continue;
        }

        
        if (isalpha(ch)) {
            lexeme = ch;
            while (file.get(ch) && (isalnum(ch) || ch == '_')) {
                lexeme += ch;
            }
            file.unget(); 
            
            TokenType type;
            if (keywords.count(lexeme)) {
                if(lexeme=="int"){
                 type = INT;
                 lexeme='^';
                }
                 else if (lexeme=="char")
                 {
                    type=CHAR;
                    lexeme='^';
                 }
                 else if (lexeme=="while")
                 {
                    type=WHILE;
                    lexeme='^';
                 }
                 else if (lexeme=="if")
                 {
                    type=IF;
                    lexeme='^';
                 }
                 else{
                    type=KEYWORD;
                 }

            } 
            else {
                  type = IDENTIFIER;
            }

            tokens.push_back({type, lexeme, line});
            continue;
        }

      
        if (isdigit(ch)) {
            lexeme = ch;
            while (file.get(ch) && isdigit(ch)) {
                lexeme += ch;
            }
            file.unget();
            tokens.push_back({INT_LITERAL, lexeme, line});
            continue;
        }

        
        if (ch == '\'') {
            lexeme = ch;
            if (file.get(ch)) {
                lexeme += ch;
                if (file.get(ch) && ch == '\'') {
                    lexeme += ch;
                    tokens.push_back({CHAR_LITERAL, lexeme, line});
                } else {
                    tokens.push_back({ERROR, lexeme, line});
                    file.unget();
                }
            } 
            else {
                tokens.push_back({ERROR, lexeme, line});
            }
            continue;
        }

       
        if (ch == '=' || ch == '!' || ch == '<' || ch == '>') {
            lexeme = ch;
            if (file.get(ch) && ch == '=') {
                lexeme += ch;
                if (lexeme =="<=")
                {
                    lexeme="LE";
                    
                }
                else if (lexeme==">=")
                {
                    lexeme="GE";
                }
                
                else if (lexeme=="!=")
                {
                    lexeme="NE";
                }
                else
                {
                    lexeme="EQ";
                }

                tokens.push_back({REL_OP, lexeme, line});
            } else {
                file.unget();
                if (lexeme =="<")
                {
                    lexeme="LT";
                }
                else if (lexeme==">")
                {
                    lexeme="GT";
                }
                else{}
                TokenType type ;
                if (lexeme == "=") 
                {
                    type=ASSIGN;
                } 
                else{ 
                    type=REL_OP;
                }

                tokens.push_back({type, lexeme, line});
            }
            continue;
        }
        if (ch == '/' && file.peek() == '/') {
            while (file.get(ch) && ch != '\n'); 
            line++;
            continue;
        }
        
        if (ch == '/' && file.peek() == '*') {
            file.get(); 
            while (file.get(ch)) {
                if (ch == '*' && file.peek() == '/') {
                    file.get();
                    break;
                }
                if (ch == '\n') line++; 
            }
            continue;
        }
/**/

        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            lexeme = ch;
            tokens.push_back({ARITH_OP, lexeme, line});
            continue;
        }
       
       

        
        if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '{' || ch == '}') {
            lexeme = ch;
            tokens.push_back({PUNCTUATION, lexeme, line});
            continue;
        }

        
        tokens.push_back({ERROR, string(1, ch), line});
    }

    
    writeTokensToFile(output, tokens);
}

int main() {
    string inputFile = "input.txt";
    string outputFile = "output.txt";

    gettoken(inputFile, outputFile);
    cout<<"token is written in the file "<<endl;

   
    return 0;
}
