#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

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
        case PLUS: case MINUS: case DIV: case MULT: return "^";
        case SEMI: case LPAREN: case RPAREN: case LBRACE: case RBRACE: return "^";
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

class Lexer {
public:
    vector<Token> tokens;
    int index = 0;

    Token peek() {
        if (index < tokens.size()) return tokens[index];
        return { END, "", 0 };
    }

    Token getNextToken() {
        if (index < tokens.size()) return tokens[index++];
        return { END, "", 0 };
    }

    void match(TokenType expected) {
        if (peek().type == expected)
            getNextToken();
        else {
            exit(1);
        }
    }

    void printtoken() {
        for (const auto &token : tokens) {
            if (tokenToString(token.type) == "ERROR") {
                cout << "('" << token.lexeme << "', " << tokenToString(token.type) << " at line " << token.line << ")";
            } else {
                cout << "(" << token.lexeme << ", " << tokenToString(token.type) << ")";
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
                    if (lexeme == "int") type = INT;
                    else if (lexeme == "char") type = CHAR;
                    else if (lexeme == "while") type = WHILE;
                    else if (lexeme == "if") type = IF;
                    else type = ELSE;
                } else {
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
                        tokens.push_back({ERROR, lexeme, line});
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
                    if (lexeme == "<=") lexeme = "LE";
                    else if (lexeme == ">=") lexeme = "GE";
                    else if (lexeme == "!=") lexeme = "NE";
                    else lexeme = "EQ";
                    tokens.push_back({RO, lexeme, line});
                } else {
                    file.unget();
                    if (lexeme == "<") lexeme = "LT";
                    else if (lexeme == ">") lexeme = "GT";
                    TokenType type;
                    if (lexeme == "=") type = ASSIGN;
                    else type = RO;
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

            if (ch == '+') {
                lexeme = ch;
                tokens.push_back({PLUS, lexeme, line});
                continue;
            }
            if (ch == '-') {
                lexeme = ch;
                tokens.push_back({MINUS, lexeme, line});
                continue;
            }
            if (ch == '*') {
                lexeme = ch;
                tokens.push_back({MULT, lexeme, line});
                continue;
            }
            if (ch == '/') {
                lexeme = ch;
                tokens.push_back({DIV, lexeme, line});
                continue;
            }

            if (ch == ';') {
                lexeme = ch;
                tokens.push_back({SEMI, lexeme, line});
                continue;
            }
            if (ch == '(') {
                lexeme = ch;
                tokens.push_back({LPAREN, lexeme, line});
                continue;
            }
            if (ch == ')') {
                lexeme = ch;
                tokens.push_back({RPAREN, lexeme, line});
                continue;
            }
            if (ch == '{') {
                lexeme = ch;
                tokens.push_back({LBRACE, lexeme, line});
                continue;
            }
            if (ch == '}') {
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
        vector<string> tac; 
        int temp_count = 0; 
        int label_count = 0; 
        int line_number = 1;
    
        // Structure to hold inherited and synthesized attributes
        struct Attributes {
            string V; 
            string label; 
            string lexeme; 
        };
        
    
        
        void emit(const string& instruction) {
            string numbered_instruction =  instruction;
            tac.push_back(numbered_instruction);
            cout << numbered_instruction << endl;


            ofstream outfile("output.txt", ios::app);  // ios::app means append mode
            if (outfile.is_open()) {
                outfile << numbered_instruction << endl;
                outfile.close();
            } else {
                cerr << "Unable to open file for writing" << endl;
            }
        }
        
        string new_temp() {
            return "t" + to_string(temp_count++);
        }
    
        
        string new_label() {
            return "L" + to_string(label_count++);
        }
    
    public:
        Parser(Lexer lex) : lexer(lex) {}
    
        void prog() {
            Attributes p;
            Decl(p);
            if (lexer.peek().type != END) {
                cerr << "Syntax error: Unexpected token after valid program" << endl;
                exit(1);
            }
            
            if (p.V!="")
              emit(p.V + " = " + p.V); 
        }
    
        void Decl(Attributes& r) {
            if (lexer.peek().type == ID || lexer.peek().type == INT || lexer.peek().type == CHAR || 
                lexer.peek().type == IF || lexer.peek().type == WHILE) {
                Attributes s, r1;
                stmt(s);
                Decl(r1);
                line_number++;
                
                
                if(r1.V !="0"&&r1.V!="")
                {
                    r.V = new_temp();
                
                emit(r.V + " = " + s.V + " + " + r1.V);
                }
                
            } else {
                r.V = "0"; 
            }
        }
    
        void stmt(Attributes& s) {
            Token t = lexer.peek();
            if (t.type == INT || t.type == CHAR) {
                Attributes d;
                decls(d);
                
                s.V = d.V;
                emit(s.V + " = " + d.V);
            } else if (t.type == ID) {
                Attributes a;
                Assignm(a);
               
                s.V = a.V;
                
                size_t pos = a.V.find(" = ");
                if (pos != string::npos) {
                    s.V = a.V.substr(pos + 3);
                    size_t semi_pos = s.V.find(";");
                    if (semi_pos != string::npos) {
                        s.V = s.V.substr(0, semi_pos); 
                    }
                }
                s.lexeme = a.lexeme; 
            } else if (t.type == IF) {
                Attributes e;
                ifstmt(e);
                s.V = e.V;
                s.lexeme = e.lexeme;
            } else if (t.type == WHILE) {
                Attributes w;
                whilestmt(w);
                s.V = w.V;
                s.lexeme = w.lexeme;
               
            } else {
                cerr << "Syntax error: Expected a statement, but got " << t.lexeme << endl;
                exit(1);
            }
        }
    
        void decls(Attributes& d) {
            Attributes t, in;
            type(t);
            Token id = lexer.peek();
            lexer.match(ID);
            IN(in);
            lexer.match(SEMI);
            
            d.V = new_temp();
            emit(d.V + " = " + t.V + " " + id.lexeme + " " + in.V + ";");
        }
    
        void type(Attributes& t) {
            if (lexer.peek().type == INT) {
                lexer.match(INT);
                
               
            } else if (lexer.peek().type == CHAR) {
                lexer.match(CHAR);
               
                t.V = "char";
                
            } else {
                cerr << "Syntax error: Expected type specifier (int or char)" << endl;
                exit(1);
            }
        }
    
        void IN(Attributes& in) {
            if (lexer.peek().type == ASSIGN) {
                lexer.match(ASSIGN);
                Attributes lit;
                LITERAL(lit);
                
                in.V = lit.V;
                emit(in.V + " = " + lit.V);
            } else {
                in.V = "";
            }
        }
    
        void LITERAL(Attributes& lit) {
            if (lexer.peek().type == CHAR_LITERAL) {
                Token cl = lexer.peek();
                lexer.match(CHAR_LITERAL);
                lit.V = cl.lexeme;
            } else if (lexer.peek().type == INT_LITERAL) {
                Token num = lexer.peek();
                lexer.match(INT_LITERAL);
                lit.V = num.lexeme;
            } else {
                cerr << "Syntax error: Expected literal (char or num)" << endl;
                exit(1);
            }
        }
    
        void Assignm(Attributes& a) {
            Token id = lexer.peek();
            lexer.match(ID);
            lexer.match(ASSIGN);
            Attributes e;
            Expr(e);
            lexer.match(SEMI);
          
            a.V = e.V; 
            a.lexeme = id.lexeme;
            emit(a.lexeme + " = " + a.V);
        }
    
        void ifstmt(Attributes& i) {
            lexer.match(IF);
            lexer.match(LPAREN);
            Attributes be;
            IWstm(be); 
            lexer.match(RPAREN);
    
            
            string lab1=new_label();

    
            
            emit("if " + be.V + " goto " + lab1);
    
           
            string lab2=new_label();

            emit( " goto " + lab2);
    
            lexer.match(LBRACE);
            Attributes d;
            emit(lab1+":");
            Decl(d); 
            lexer.match(RBRACE);
    
           
            string labelend=new_label();
            emit("goto " + labelend);

    
            Attributes e;
            emit(  lab2+":");
            Else(e); // "else" part: max = y
    
            // 5) max = y
           // emit(e.lexeme + " = " + e.V); // Use e.lexeme (e.g., "max") and e.V (e.g., "y")
    
            // 6) halt
           // emit("halt");
    
            i.V = "max";
            i.lexeme = d.lexeme; 
        }
    
        void Else(Attributes& e) {
            if (lexer.peek().type == ELSE) {
                lexer.match(ELSE);
                Attributes e1;
                Else_(e1);
                e.V = e1.V;
                e.lexeme = e1.lexeme;
            } else {
                e.V = "0"; 
                e.lexeme = "";
            }
        }
    
        void Else_(Attributes& e1) {
            if (lexer.peek().type == IF) {
                Attributes i;
                ifstmt(i);
                e1.V = i.V;
                e1.lexeme = i.lexeme;
                emit(e1.V + " = " + i.V);
            } else {
                lexer.match(LBRACE);
                Attributes s;
                stmt(s);
                lexer.match(RBRACE);
                e1.V = s.V; 
                e1.lexeme = s.lexeme; 
            }
        }
    
        void IWstm(Attributes& be) {
            Token id1 = lexer.peek();
            lexer.match(ID);
            Token ro = lexer.peek();
            lexer.match(RO);
            Token id2;
            if (lexer.peek().type == ID) {
                id2 = lexer.peek();
                lexer.match(ID);
            } else {
                id2 = lexer.peek();
                lexer.match(INT_LITERAL);
            }
            
            be.V = id1.lexeme + " " + convertto(ro.lexeme) + " " + id2.lexeme;
        }
    
        
        void Expr(Attributes& e) {
            Attributes a, b;
            A(a);
            B(a, b); // Pass 'a' as the left operand, 'b' will hold the result of the rest
           
            e.V = b.V.empty() ? a.V : b.V;
        }
    
        void B(Attributes& left, Attributes& b) {
            if (lexer.peek().type == PLUS || lexer.peek().type == MINUS) {
                Token op = lexer.peek();
                if (op.type == PLUS) {
                    lexer.match(PLUS);
                } else {
                    lexer.match(MINUS);
                }
                Attributes a;
                A(a);
                
                string temp = new_temp();
                emit(temp + " = " + left.V + " " + (op.type == PLUS ? "+" : "-") + " " + a.V);
                // Continue processing the rest of the expression with the result as the new left operand
                Attributes next;
                next.V = temp;
                B(next, b);
            } else {
                b.V = left.V; // No more operators, propagate the left operand
            }
        }
    
        void A(Attributes& a) {
            Attributes c, d;
            C(c);
            D(c, d);
            // A → C D { A.V := C.V if D.V is empty, otherwise D.V }
            a.V = d.V.empty() ? c.V : d.V;
        }
    
        void D(Attributes& left, Attributes& d) {
            if (lexer.peek().type == MULT || lexer.peek().type == DIV) {
                Token op = lexer.peek();
                if (op.type == MULT) {
                    lexer.match(MULT);
                } else {
                    lexer.match(DIV);
                }
                Attributes c;
                C(c);
              
                string temp = new_temp();
                emit(temp + " = " + left.V + " " + (op.type == MULT ? "*" : "/") + " " + c.V);
                
                Attributes next;
                next.V = temp;
                D(next, d);
            } else {
                d.V = left.V; // No more operators, propagate the left operand
            }
        }
    
        void C(Attributes& c) {
            if (lexer.peek().type == LPAREN) {
                lexer.match(LPAREN);
                Attributes e;
                Expr(e);
                lexer.match(RPAREN);
                // Y → (E) { Y.V := “(” + E.V + “)”³ }
                c.V = new_temp();
                emit(c.V + " = ( " + e.V + " )");
            } else if (lexer.peek().type == ID) {
                Token id = lexer.peek();
                lexer.match(ID);
                // H → id { H.V := id.lex³ }
                c.V = id.lexeme;
            } else if (lexer.peek().type == INT_LITERAL) {
                Token num = lexer.peek();
                lexer.match(INT_LITERAL);
                c.V = num.lexeme;
            } else {
                cerr << "Syntax error: Expected an expression" << endl;
                exit(1);
            }
        }
    
        void whilestmt(Attributes& w) {
            lexer.match(WHILE);
            lexer.match(LPAREN);
            Attributes be;
            IWstm(be);
            lexer.match(RPAREN);
            string lab1=new_label();
            emit(lab1+":");
            string lab2=new_label();
            emit("if " + convertto(be.V) + " goto " + lab2);
            //string label3 = to_string(line_number + 4);
            emit("goto  end loop ");
            emit(lab2+":");
            //emit(label2 + ":");
            lexer.match(LBRACE);
            Attributes d;
            Decl(d);
            emit("  ");
            lexer.match(RBRACE);
            emit("goto " + lab1);
            //emit(label3 + ":");
            w.V = new_temp();
           // emit(w.V + " = " + d.V);
        }
        string convertto(string r)
{
                if (r == "LE") r = "<=";
                    else if (r == "GE") r = ">=";
                    else if (r == "NE") r = "!=";
                    else if (r == "EQ") r="==";
                     
                    //tokens.push_back({RO, lexeme, line});
                
                else
                {
                    //file.unget();
                    if (r == "LT") r= " <";
                    else if (r == "GT") r = ">";
                    //TokenType type;
                    else
                    {int k=0;}
                    //if (lexeme == "=") type = ASSIGN;
                    //else type = RO;
                }
                return r;
}
    };


int main() {
    Lexer lexer;
    lexer.gettoken("input.txt");
    Parser parser(lexer);
    lexer.printtoken();
    cout << "\nThree-Address Code:\n";
    try {
        parser.prog();
        cout << "Parsing completed successfully!" << endl;
    } catch (...) {
        cout << "Parsing failed due to syntax errors." << endl;
    }
    return 0;
}