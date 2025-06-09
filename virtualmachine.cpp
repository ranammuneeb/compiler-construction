#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>
#include <map>
#include <sstream>

using namespace std;
ofstream file("output.txt",ios::trunc);

// Enum for Token Types
enum TokenType {
    ID, INT_LITERAL, INT, CHAR, IF, ELSE, WHILE, 
    IN, OUT, // Added for input/output
    PLUS, MINUS, MULT, DIV, ASSIGN, SEMI, CHAR_LITERAL,
    LPAREN, RPAREN, LBRACE, RBRACE, RO, CH, END, ERROR
};
struct TypedExpr {
    std::string value; // e.g., temp variable or literal
    std::string type;  // "int" or "char"
};
bool isInteger(const std::string& s) {
    if (s.empty()) return false;

    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') start = 1;

    for (size_t i = start; i < s.length(); ++i) {
        if (!std::isdigit(s[i])) return false;
    }
    return true;
}
bool isIdentifier(const string& token) {
    return isalpha(token[0]); // crude check: starts with a letter
}



bool isCharLiteral(const std::string& s) {
    // Must be 3 characters like 'a' or 4+ like '\n' (escaped)
    if (s.length() == 3 && s[0] == '\'' && s[2] == '\'') {
        return true;  // e.g., 'a'
    }
    if (s.length() == 4 && s[0] == '\'' && s[1] == '\\' && s[3] == '\'') {
        return true;  // e.g., '\n'
    }
    return false;
}

// Convert TokenType to string for output
string tokenToString(TokenType type) {
    switch (type) {
        case INT: return "^";
        case CHAR: return "^";
        case WHILE: return "^";
        case IF: return "^";
        case ELSE: return "^";
        case IN: return "^"; // Added
        case OUT: return "^"; // Added
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

// Token structure
struct Token {
    TokenType type;
    string lexeme;
    int line;
};

// Keyword set
unordered_set<string> keywords = {"int", "char", "if", "else", "while", "in", "out"};

// Quadruple structure for TAC
struct Quad {
    string op;
    string arg1;
    string arg2;
    string result;
};

// Symbol table entry
struct Symbol {
    string name;
    string type; // e.g., "int", "char"
    string value; // string representation of value
};

// Lexer class
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
            cerr << "Syntax error: Expected " << tokenToString(expected) << " at line " << peek().line << endl;
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
        cout << endl;
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
                    else if (lexeme == "else") type = ELSE;
                    else if (lexeme == "in") type = IN; // Added
                    else if (lexeme == "out") type = OUT; // Added
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
        file.close();
    }
};

// Parser class
class Parser {
private:
    Lexer lexer;
    
    map<string, Symbol> symbol_table; // Symbol table
    int temp_count = 0;
    int label_count = 0;
    vector<string> tac; 

    void emit(const string& instruction) {
        string numbered_instruction =  instruction;
        tac.push_back(numbered_instruction);
        //cout << numbered_instruction << endl;


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

    void add_to_symbol_table(const string& name, const string& type, const string& value = "0") {
        symbol_table[name] = {name, type, value};
        cout<<symbol_table[name].name<<" ";
        cout<<symbol_table[name].type<<"";
        cout<<symbol_table[name].value<<"";
        cout<<endl;

    }

    bool checkExprType(string expr, string expectedType) {
        std::istringstream iss(expr);
        string token;
        while (iss >> token) {
            // If it's a variable, check its type
            if (isIdentifier(token)) {
                auto it = symbol_table.find(token);
                if (it == symbol_table.end()) {
                    cerr << "Undeclared variable '" << token << "'\n";
                    exit(EXIT_FAILURE);
                }
                if (it->second.type != expectedType) {
                    cerr << "Type error: Incompatible type for variable '" << token << "' in expression\n";
                    exit(EXIT_FAILURE);
                }
            }
            if(isCharLiteral(token))
            {
                auto it = symbol_table.find(token);
               
                if (it->second.type != expectedType) {
                    cerr << "Type error: Incompatible type for variable '" << token << "' in expression expected type was"<<expectedType<<endl;;
                    exit(EXIT_FAILURE);
                }
            }
            if (isInteger(token))
            {
                auto it = symbol_table.find(token);
               
                if (it->second.type != expectedType) {
                   // cerr << "Type error: Incompatible type  '" << token << "' in expression expected type was"<<expectedType<<endl;;
                   // exit(EXIT_FAILURE);
                }
            }
        }
        return true;
    }

public:
    Parser(Lexer lex) : lexer(lex) {}

    
    map<string, Symbol>get_symbol_table()  { return symbol_table; }

    void prog() {
        Decl();
        if (lexer.peek().type != END) {
            cerr << "Syntax error: Unexpected token after valid program" << endl;
            exit(1);
        }
    }

    void Decl() {
        while (lexer.peek().type == ID || lexer.peek().type == INT || lexer.peek().type == CHAR || 
               lexer.peek().type == IF || lexer.peek().type == WHILE || lexer.peek().type == IN|| lexer.peek().type == OUT) {
            stmt();
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
        } else if (t.type == IN) {
            instmt();
        } else if (t.type == OUT) {
            outstmt();
        } else {
            cerr << "Syntax error: Expected a statement, but got " << t.lexeme << endl;
            exit(1);
        }
    }

    void decls() {
        string typ= type();
        Token id = lexer.peek();
        lexer.match(ID);
        INTE(typ, id.lexeme);
        lexer.match(SEMI);
       // add_to_symbol_table(id.lexeme, typ);
      
       if (symbol_table.find(id.lexeme) == symbol_table.end()) {
        add_to_symbol_table(id.lexeme, typ);
    }
    


    }

    string type() {
        if (lexer.peek().type == INT) {
            lexer.match(INT);
            return "int";
        } else if (lexer.peek().type == CHAR) {
            lexer.match(CHAR);
            return "char";
        } else {
            cerr << "Syntax error: Expected type specifier (int or char)" << endl;
            exit(1);
        }
        return "";
    }

    void INTE(const string& type, const string& id) {
        if (symbol_table.find(id) != symbol_table.end()) {
            cerr<<"already declared   "<<id<<endl;
            exit(1);
        }
        if (lexer.peek().type == ASSIGN) {
            lexer.match(ASSIGN);
            string lit = LITERAL();
            emit(id+" = " +lit);
            if(type=="int")
            {
                if(isCharLiteral(lit))
                {
                    cerr << "cannot assign charac to int '" << lit<< "'\n";
                    exit(EXIT_FAILURE);
                }
            }
            if(type=="char")
            {
                if(isInteger(lit))
                {
                    cerr << "cannot assign integer to char '" << lit<< "'\n";
                    exit(EXIT_FAILURE);
                }
            }

            add_to_symbol_table(id, type, lit);
        }
    }

    string LITERAL() {
        if(lexer.peek().type==MINUS)
        {
            lexer.match(MINUS);
            if (lexer.peek().type == INT_LITERAL) {
                Token num = lexer.peek();
                lexer.match(INT_LITERAL);
                return "-"+num.lexeme;
            }

        }
        else if (lexer.peek().type == CHAR_LITERAL) {
            Token cl = lexer.peek();
            lexer.match(CHAR_LITERAL);
            return cl.lexeme;
        } else if (lexer.peek().type == INT_LITERAL) {
            Token num = lexer.peek();
            lexer.match(INT_LITERAL);
            return num.lexeme;
        } else {
            cerr << "Syntax error: Expected literal (char or num)" << endl;
            exit(1);
        }
        return "";
    }

    void Assignm() {
        //Token id = lexer.peek();
        //string a=id.lexeme;
        
        // lexer.match(ID);
        // lexer.match(ASSIGN);
        // string e = Expr();
        // lexer.match(SEMI);
        // emit( id.lexeme+" = "+ e);
        // if (symbol_table.find(id.lexeme) != symbol_table.end()) {
        //     symbol_table[id.lexeme].value = e;
        // }
        Token id = lexer.peek();
        string a = id.lexeme;

        lexer.match(ID);
        lexer.match(ASSIGN);
        TypedExpr expr = Expr(); // generate/compute the right-hand side expression
        lexer.match(SEMI);
       
        // Emit the 3-address code
        emit(id.lexeme + " = " + expr.value);
        //cout<<symbol_table.find(id.lexeme);
        // Check if id exists in the symbol table
        auto it = symbol_table.find(id.lexeme);
        if (it != symbol_table.end()) {
            if (it->second.type == "int") {
                checkExprType(expr.value, "int");  // e.g., "a + b"
            }
            else if (it->second.type == "char") {
                checkExprType(expr.value, "char");
            }
            symbol_table[id.lexeme].value = expr.value;
        } else {
            cerr << "Undeclared variable '" << id.lexeme << "'\n";
            exit(EXIT_FAILURE);
        } 
    }

    void instmt() {
        lexer.match(IN);
        Token id = lexer.peek();
        lexer.match(ID);
        lexer.match(SEMI);
        emit("in "+ id.lexeme);
    }

    void outstmt() {
        lexer.match(OUT);
        Token id = lexer.peek();
        lexer.match(ID);
        lexer.match(SEMI);
        emit("out "+id.lexeme);
    }

    void ifstmt() {
        lexer.match(IF);
        lexer.match(LPAREN);
        string be; // Store relational expression
        IWstm(be); // Pass be to capture expression
        lexer.match(RPAREN);

        string lab1 = new_label();
        emit("\n");
        emit("if "+ be+" goto "+ lab1);
        string lab2 = new_label();
        emit("goto "+lab2);

        lexer.match(LBRACE);
        emit( lab1+ " :");
        Decl();
        lexer.match(RBRACE);

        string lab_end = new_label();
        emit("goto "+ lab_end);
        emit(  lab2+" :");

        Else();
        emit(lab_end +" :\n");
        
    }
    void Else() {
        if (lexer.peek().type == ELSE) {
            lexer.match(ELSE);
            if (lexer.peek().type == IF) {
                ifstmt();
            } else {
                lexer.match(LBRACE);
                Decl();
                lexer.match(RBRACE);
            }
        }
    }
    void IWstm(string& be) 
    {
        Token id1 = lexer.peek();
        lexer.match(ID);
        Token ro = lexer.peek();
        lexer.match(RO);
        Token id2 = lexer.peek();
        if (lexer.peek().type == ID) {
            lexer.match(ID);
        } else {
            lexer.match(INT_LITERAL);
        }
        be = id1.lexeme + " " + convertto(ro.lexeme) + " " + id2.lexeme;
    }

    TypedExpr Expr() {
        TypedExpr a = A();
        return B(a);
    }
    

    TypedExpr B(const TypedExpr& left) {
        if (lexer.peek().type == PLUS || lexer.peek().type == MINUS) {
            Token op = lexer.peek();
            lexer.match(op.type);
            TypedExpr right = A();
    
            // Type check
            if (left.type != "int" || right.type != "int") {
                std::cerr << "Type error: Arithmetic operation requires integer operands\n";
                exit(EXIT_FAILURE);
            }
    
            std::string temp = new_temp();
            add_to_symbol_table(temp, "int");
            emit(temp + " = " + left.value + " " + (op.type == PLUS ? "+" : "-") + " " + right.value);
            return B({temp, "int"});
        }
        return left;
    }
    

    TypedExpr A() {
        TypedExpr c = C();
        return D(c);
    }

    TypedExpr C() {
        if (lexer.peek().type==MINUS)
        {
            lexer.match(MINUS);
            if (lexer.peek().type == INT_LITERAL) {
                Token num = lexer.peek();
                lexer.match(INT_LITERAL);
                return {"-"+num.lexeme, "int"};
            }
        }
        if (lexer.peek().type == LPAREN) {
            lexer.match(LPAREN);
            TypedExpr expr = Expr();
            lexer.match(RPAREN);
    
            std::string temp = new_temp();
            add_to_symbol_table(temp, expr.type);
            emit(temp + " = ( " + expr.value + " )");
            return {temp, expr.type};
        }
    
        if (lexer.peek().type == ID) {
            Token id = lexer.peek();
            lexer.match(ID);
    
            auto it = symbol_table.find(id.lexeme);
            if (it == symbol_table.end()) {
                std::cerr << "Error: Variable '" << id.lexeme << "' used before declaration\n";
                exit(EXIT_FAILURE);
            }
    
            return {id.lexeme, it->second.type};
        }
    
        if (lexer.peek().type == INT_LITERAL) {
            Token num = lexer.peek();
            lexer.match(INT_LITERAL);
            return {num.lexeme, "int"};
        }
    
        if (lexer.peek().type == CHAR_LITERAL) {
            Token ch = lexer.peek();
            lexer.match(CHAR_LITERAL);
            return {ch.lexeme, "char"};
        }
    
        std::cerr << "Syntax error: Expected an expression\n";
        exit(EXIT_FAILURE);
    }
    TypedExpr D(const TypedExpr& left) {
        if (lexer.peek().type == MULT || lexer.peek().type == DIV) {
            Token op = lexer.peek();
            lexer.match(op.type);
            TypedExpr right = C();
    
            // Type check
            if (left.type != "int" || right.type != "int") {
                std::cerr << "Type error: Multiplicative operation requires integer operands\n";
                exit(EXIT_FAILURE);
            }
    
            std::string temp = new_temp();
            add_to_symbol_table(temp, "int");
            emit(temp + " = " + left.value + " " + (op.type == MULT ? "*" : "/") + " " + right.value);
            return D({temp, "int"});
        }
        return left;
    }
    
    

    void whilestmt() {
        string lab1 = new_label();
        emit(lab1+" :");
        lexer.match(WHILE);
        lexer.match(LPAREN);
        string be; // Store relational expression
        IWstm(be); // Pass be to capture expression
        lexer.match(RPAREN);
        string lab2 = new_label();
        //emit("\n");
        emit("if "+be+ " goto "+ lab2);
        string endlabe=new_label();
        emit("goto "+endlabe);
        emit( lab2+" :");
        lexer.match(LBRACE);
        Decl();
        lexer.match(RBRACE);
        emit("goto " + lab1);
        emit(endlabe+" :");
        emit("\n");
    }

    string convertto(string r) {
        if (r == "LE") return "<=";
        else if (r == "GE") return ">=";
        else if (r == "NE") return "!=";
        else if (r == "EQ") return "==";
        else if (r == "LT") return "<";
        else if (r == "GT") return ">";
        return r;
    }
};
void parse_TAC_from_file(const string& filename, vector<Quad>& quads) {
    ifstream infile(filename);
    string line;

    while (getline(infile, line)) {
        istringstream iss(line);
        string token;

        if (line.empty()) continue;

        // --- Skip labels like "L1 :" ---
        if (line.back() == ':' || (line.substr(0, 2) == "L" && line.find(':') != string::npos)) {
            string labe;
            iss>>labe;
            quads.push_back({"label","","",labe});
        }

        // --- Format: if a == b goto L0 ---
        if (line.substr(0, 2) == "if") {
            string arg1, op, arg2, goto_kw, label;
            iss >> token >> arg1 >> op >> arg2 >> goto_kw >> label;
            quads.push_back({op, arg1, arg2, label});
            //quads.push_back({"goto", "", "", label});
        }

        // --- Format: goto L1 ---
        else if (line.substr(0, 4) == "goto") {
            string goto_kw, label;
            iss >> goto_kw >> label;
            quads.push_back({"goto", "", "", label});
        }

        // --- Format: in x / out x ---
        else if (line.substr(0, 2) == "in" || line.substr(0, 3) == "out") {
            string op, var;
            iss >> op >> var;
            quads.push_back({op, var, "", ""});
        }
        // else if (line.find('=') != string::npos && line.find('-') != string::npos &&
        //         (line.find('+') == string::npos || line.find('*') == string::npos || line.find('/') == string::npos)) {
        //     string result, eq, arg1, op, arg2;
        //     iss >> result >> eq >> arg1 >> op >> arg2;
        //     quads.push_back({eq, arg1, "", result});
        // }
        // --- Format: x = y op z ---
        // --- Format: x = y OR x = y op z OR x = -1 ---
        else if (line.find('=') != string::npos) {
            string result, eq, arg1, op, arg2;
            iss >> result >> eq >> arg1;

            // Handle negative constant (e.g., x = -1)
            if (arg1[0] == '-' && isdigit(arg1[1])) {
                quads.push_back({eq, arg1, "", result});
            } 
            // Try to read an operator
            else if (iss >> op) {
                if (op == "+" || op == "-" || op == "*" || op == "/") {
                    iss >> arg2;
                    quads.push_back({op, arg1, arg2, result});
                } else {
                    // Not an operator, treat 'op' as part of the assignment (e.g., variable like "-x")
                    arg1 += op;
                    quads.push_back({eq, arg1, "", result});
                }
            } 
            // Simple assignment
            else {
                quads.push_back({eq, arg1, "", result});
            }
        }


        else {
            //cerr << "Warning: Unrecognized line in TAC: " << line << endl;
        }
    }

    infile.close();
}

void execute_VM(vector<Quad>& quads, map<string, Symbol>& symtab) {
    map<string, int> memory;
    int pc = 0;
    map<string, int> labelMap;

    // Preprocess labels
    for (int i = 0; i < quads.size(); i++) {
        if (quads[i].op == "label") {
            labelMap[quads[i].result] = i;
        }
    }

    while (pc < quads.size()) {
        Quad q = quads[pc];
        
        if (q.op == "=") {
            int value;
            if (isdigit(q.arg1[0]) || (q.arg1[0] == '-' && isdigit(q.arg1[1]))) {
                value = stoi(q.arg1);
            } else {
                value = memory[q.arg1];
            }
        
            memory[q.result] = value;
            symtab[q.result].value = to_string(value);
        }

        else if (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (q.op == "+") memory[q.result] = left + right;
            else if (q.op == "-") memory[q.result] = left - right;
            else if (q.op == "*") memory[q.result] = left * right;
            else if (q.op == "/") memory[q.result] = (right != 0 ? left / right : 0);
            symtab[q.result].value = to_string(memory[q.result]);
        }

        else if (q.op == "==") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left == right) pc = labelMap[q.result];
        }

        else if (q.op == "!=") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left != right) pc = labelMap[q.result];
        }

       

        else if (q.op == "<=") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left <= right) pc = labelMap[q.result];
        }

       

        else if (q.op == ">=") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left >= right) pc = labelMap[q.result];
        }

        else if (q.op == "goto") {
            pc = labelMap[q.result];
           // continue;
        }

        else if (q.op == "in") {
            if(symtab.find(q.arg1)!=symtab.end())
            {
                int input_val;
            cout << "Enter value for " << q.arg1 << ": ";
            cin >> input_val;
            memory[q.arg1] = input_val;
            symtab[q.arg1].value = to_string(input_val);
        
            }
            else{
                cout<<q.arg1<<"is not declared"<<endl;
            }
        }
        
        else if (q.op == "<") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left < right) pc = labelMap[q.result];
        }
        
        else if (q.op == "out") {
            if (symtab.find(q.arg1) != symtab.end()) {
                cout << q.arg1 << " = " << memory[q.arg1] << endl;
                symtab[q.arg1].value = to_string(memory[q.arg1]);  
            } else {
                cout << q.arg1 << " is not declared" << endl;
            }
            
        }
        else if (q.op == ">") {
            int left = isdigit(q.arg1[0]) ? stoi(q.arg1) : memory[q.arg1];
            int right = isdigit(q.arg2[0]) ? stoi(q.arg2) : memory[q.arg2];
            if (left > right) pc = labelMap[q.result];
        }
        

        pc++;
    }
}





int main() {
    Lexer lexer;
    lexer.gettoken("file.txt");
    Parser parser(lexer);
    lexer.printtoken();
    //cout << "\nThree-Address Code (Quadruples):\n";
    try {
        parser.prog();
        cout << "\nParsing completed successfully!" << endl;
        cout<<"three address code written in the file output.tx"<<endl;
        

        //Execute the virtual machine
         vector<Quad> quads;
         // Display the quads
    //      cout<<"quadruple"
    parse_TAC_from_file("output.txt", quads);
    // for (const auto& q : quads) {
    //     cout << "(" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")\n";
    // }
        
        cout << "\nExecuting Virtual Machine:\n";
        
      
       

    
    cout << "\n--- Virtual Machine Execution ---\n";
    cout<<"output of the code"<<endl;
    map<string, Symbol> symtab=parser.get_symbol_table();
    execute_VM(quads,symtab) ;

    cout << "\n\nSymbol Table:\n";
        for (const auto& entry : symtab) {
            cout << "Name: " << entry.second.name 
                 << ", Type: " << entry.second.type 
                 << ", Value: " << entry.second.value << endl;
        }
        // Print symbol table
       
    } catch (...) {
        cout << "Parsing failed due to syntax errors." << endl;
    }
    return 0;
}