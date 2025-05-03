#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include<unordered_set>

using namespace std;

enum TokenType {
    ID, INT_LITERAL, INT, CHAR, IF, ELSE, WHILE, 
    PLUS, MINUS, MULT, DIV, ASSIGN, SEMI, CHAR_LITERAL,
    LPAREN, RPAREN, LBRACE, RBRACE, RO, CH, END, ERROR
};
string tokenToString(TokenType type) {
    switch (type) {
        case INT: return "^";
        case CHAR: return "^";
        case WHILE: return "^";
        case IF: return "^";
        case ELSE: return "^";
        case ID: return "ID";
        case INT_LITERAL: return "NUM";
        case CHAR_LITERAL: return "CL";
        case RO: return "RO";
        case ASSIGN: return "^";
        case PLUS:case MINUS :case DIV:case MULT: return "^";
        case SEMI:case LPAREN : case RPAREN : case LBRACE:case RBRACE: return "^";
        
        case ERROR: return "ERROR";
        
        default: return "UNK";
    }
}


struct Token {
    TokenType type;
    string lexeme;
    //int index;
    int line;
    
};
unordered_set<string> keywords = {"int", "char", "if", "else", "while"};
class Lexer {
public:
    vector<Token> tokens;
    
    int index=0;
   

   
    Token peek() {
        if (index < tokens.size()) return tokens[index];
        return { END, "" ,0}; // End of input
    }

    Token getNextToken() {
        if (index < tokens.size()) return tokens[index++];
        return { END, "",0 };
    }

    void match(TokenType expected) {
        if (peek().type == expected)
            getNextToken();
        else {
           
            exit(1);
        }
    }
    void printtoken()
    {
        for (const auto &token : tokens) {
        
            
            if (tokenToString(token.type)=="ERROR")
            {
                cout << "('" <<token.lexeme   << "',  " << tokenToString(token.type) << "at line  "<<token.line<<")" ;
            }
            else 
            {
              cout << "(" <<token.lexeme   << ",  " << tokenToString(token.type) << ")" ;
            }
        
            
        }
    }

    void gettoken(const string &input) {
        ifstream file(input);
        if (!file) {
            cerr << "Error opening input file!" << endl;
            return;
        }
        this->tokens.clear();
    
        //vector<Token> tokens;
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
                     type = INT;}
                     else if (lexeme=="char")
                     {
                        type=CHAR;
                     }
                     else if (lexeme=="while")
                     {
                        type=WHILE;
                     }
                     else if (lexeme=="if")
                     {
                        type=IF;
                     }
                     else{
                        type=ELSE;
                     }
    
                } 
                else {
                      type = ID;
                }
    
               this->tokens.push_back({type, lexeme, line});
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
                        tokens.push_back({ERROR,lexeme,line});
                        file.unget();
                    }
                } else {
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
    
                    tokens.push_back({RO, lexeme, line});
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
                        type=RO;
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
    
            if (ch == '+' ) {
                lexeme = ch;
                tokens.push_back({PLUS, lexeme, line});
                continue;
            }
            if (ch=='-')
            {
                lexeme = ch;
                tokens.push_back({MINUS, lexeme, line});
                continue;   
            }
            if (ch=='*')
            {
                lexeme = ch;
                tokens.push_back({MULT, lexeme, line});
                continue;   
            }
            if (ch=='/')
            {
                lexeme = ch;
                tokens.push_back({DIV, lexeme, line});
                continue;   
            }
    
            
            
             if (ch == ';')
             {
                lexeme = ch;
                tokens.push_back({SEMI, lexeme, line});
                continue;  
             }
             if (ch == '(')
             {
                lexeme = ch;
                tokens.push_back({LPAREN, lexeme, line});
                continue;  
             }
             if (ch == ')')
             {
                lexeme = ch;
                tokens.push_back({RPAREN, lexeme, line});
                continue;  
             }
             if (ch == '{')
             {
                lexeme = ch;
                tokens.push_back({LBRACE, lexeme, line});
                continue;  
             }
             if (ch == '}')
             {
                lexeme = ch;
                tokens.push_back({RBRACE, lexeme, line});
                continue;  
             }
    
            
            tokens.push_back({ERROR, string(1, ch), line});
        }
    }
};
class Parser {
    private:
        Lexer lexer;
    
    public:
        Parser(Lexer lex) : lexer(lex) {}
    
        void prog() {
            Decl();
            if (lexer.peek().type != END) {
                cerr << "Syntax error: Unexpected token after valid program" << endl;
                exit(1);
            }
        }
    
        void Decl() {
            if (lexer.peek().type == ID || lexer.peek().type == INT || lexer.peek().type == CHAR || 
                lexer.peek().type == IF || lexer.peek().type == WHILE) {
                stmt();
                Decl();
            }
        }
    
        void stmt() {
            Token t = lexer.peek();
            if (t.type == INT || t.type == CHAR) {
                decls();
            } else if (t.type == ID) {
                Assignm();
            } else if (t.type == IF) {
                ifstmt();
            } else if (t.type == WHILE) {
                whilestmt();
            } else {
                cerr << "Syntax error: Expected a statement, but got " << t.lexeme << endl;
                exit(1);
            }
        }
    
        void decls() {
            type();
            lexer.match(ID);
            IN();
            lexer.match(SEMI);
        }
    
        void type() {
            if (lexer.peek().type == INT) {
                lexer.match(INT);
            } else if (lexer.peek().type == CHAR) {
                lexer.match(CHAR);
            } else {
                cerr << "Syntax error: Expected type specifier (int or char)" << endl;
                exit(1);
            }
        }
    
        void IN() {
            if (lexer.peek().type == ASSIGN) {
                lexer.match(ASSIGN);
                LITERAL();
            }
        }
    
        void LITERAL() {
            if (lexer.peek().type == CHAR_LITERAL) {
                lexer.match(CHAR_LITERAL);
            } else if (lexer.peek().type ==INT_LITERAL) {
                lexer.match(INT_LITERAL);
            } else {
                cerr << "Syntax error: Expected literal (char or num)" << endl;
                exit(1);
            }
        }
    
        void Assignm() {
            lexer.match(ID);
            lexer.match(ASSIGN);
            Expr();
            lexer.match(SEMI);
        }
    
        void Expr() {
            A();
            B();
        }
    
        void B() {
            if (lexer.peek().type == PLUS) {
                lexer.match(PLUS);
                A();
                B();
            } else if (lexer.peek().type == MINUS) {
                lexer.match(MINUS);
                A();
                B();
            }
        }
    
        void A() {
            C();
            D();
        }
    
        void D() {
            if (lexer.peek().type == MULT) {
                lexer.match(MULT);
                C();
                D();
            } else if (lexer.peek().type == DIV) {
                lexer.match(DIV);
                C();
                D();
            }
        }
    
        void C() {
            if (lexer.peek().type == LPAREN) {
                lexer.match(LPAREN);
                Expr();
                lexer.match(RPAREN);
            } else if (lexer.peek().type == ID) {
                lexer.match(ID);
            } else if (lexer.peek().type ==INT_LITERAL) {
                lexer.match(INT_LITERAL);
            } else {
                cerr << "Syntax error: Expected an expression" << endl;
                exit(1);
            }
        }
    
        void ifstmt() {
            lexer.match(IF);
            lexer.match(LPAREN);
            IWstm();
            lexer.match(RPAREN);
            lexer.match(LBRACE);
            Decl();
            lexer.match(RBRACE);
            Else();
        }
    
        void Else() {
            if (lexer.peek().type == ELSE) {
                lexer.match(ELSE);
                Else_();
            }
            else{}
        }
        void Else_()
        {
            if (lexer.peek().type == IF) {
                ifstmt();
            } else {
                lexer.match(LBRACE);
                stmt();
                lexer.match(RBRACE);
            }
        }
    
        void IWstm() {
            lexer.match(ID);
            lexer.match(RO);
            if (lexer.peek().type == ID) {
                lexer.match(ID);
            } else {
                lexer.match(INT_LITERAL);
            }
        }
    
        void whilestmt() {
            lexer.match(WHILE);
            lexer.match(LPAREN);
            IWstm();
            lexer.match(RPAREN);
            lexer.match(LBRACE);
            Decl();
            lexer.match(RBRACE);
        }
};
    
int main() {
    

    Lexer lexer;
    lexer.gettoken("input.txt");
    Parser parser(lexer);
    lexer.printtoken();
    parser.prog();
    try {
        
        cout << "Parsing completed successfully!" << endl;
    } catch (...) {
        cout << "Parsing failed due to syntax errors." << endl;
    }

    return 0;
}