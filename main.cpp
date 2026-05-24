#include <iostream>
#include <stdexcept>  
//stdexcept是C++标准库中的一个头文件，
//包含了定义标准异常类的声明，如std::runtime_error等。
#include <string>
#include <vector>

/*
 * Recursive descent expression calculator framework.
 *
 * Grammar used in this lab:
 *
 *   E  -> T E'
 *   E' -> + T E' | - T E' | epsilon
 *   T  -> F T'
 *   T' -> * F T' | / F T' | epsilon
 *   F  -> NUM | (E)
 *
 * Function mapping:
 *
 *   parseExpression()          -> E
 *   parseExpressionPrime(...)  -> E'
 *   parseTerm()                -> T
 *   parseTermPrime(...)        -> T'
 *   parseFactor()              -> F
 */

enum class TokenType {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    LeftParen,
    RightParen,
    End
};

struct Token {
    TokenType type;
    //value用于保存数字token的数值。
    double value;
    //text用于保存token对应的原始文本（如"123"、"+"、"("等）。
    std::string text;
};

//词法分析器
class Lexer {
public:
    explicit Lexer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input_;
    //pos_是一个索引，指示当前正在分析输入字符串的位置。
    std::size_t pos_;

    bool isAtEnd() const;
    char peek() const;
    char advance();

    bool isDigit() const;

    void skipWhitespace();
    Token scanNumber();
};

//语法分析器
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    double parse();

private:
    const std::vector<Token>& tokens_;
    std::size_t pos_;

    const Token& current() const;

    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& message);

    double parseExpression();
    double parseExpressionPrime(double inheritedValue);
    double parseTerm();
    double parseTermPrime(double inheritedValue);
    double parseFactor();
};

//explicit关键字用于构造函数，表示该构造函数不能用于隐式类型转换。
Lexer::Lexer(const std::string& input)
    : input_(input), pos_(0) {
}

//tokenize函数负责将输入字符串转换为一系列Token对象，表示词法分析的结果。
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens={};
    while(!isAtEnd()){
        if(peek()==' ') skipWhitespace();
        if(!isAtEnd()){
            if(isDigit()) tokens.push_back(scanNumber());
            else{
                Token cur_token;
                switch(peek()){
                    case '+':cur_token.type=TokenType::Plus;break;
                    case '-':cur_token.type=TokenType::Minus;break;
                    case '*':cur_token.type=TokenType::Multiply;break;
                    case '/':cur_token.type=TokenType::Divide;break;
                    case '(':cur_token.type=TokenType::LeftParen;break;
                    case ')':cur_token.type=TokenType::RightParen;break;
                    default:
                        throw std::runtime_error(std::string("Unexpected character: ") + peek());
                }
                cur_token.text=advance();
                tokens.push_back(cur_token);
            }
        }
    }
    tokens.push_back({TokenType::End,0.0,""});
    return tokens;
}

//isAtEnd函数检查pos_是否已经到达输入字符串的末尾，返回一个布尔值。
bool Lexer::isAtEnd() const {
    if(pos_<input_.size()) return false;
    return true;
}

//peek函数返回当前字符，但不移动pos_，用于预先查看当前位置的字符。
char Lexer::peek() const {
    if(!isAtEnd()) return input_[pos_];
    return '\0';
}

//advance函数返回当前字符并将pos_向前移动一个位置，表示已经处理了这个字符。
char Lexer::advance() {
    if(!isAtEnd()){
        pos_++;
        return input_[pos_-1];
    }
    return '\0';
}

//skipWhitespace函数负责跳过输入中的空格。
void Lexer::skipWhitespace() {
    while(peek()==' ') pos_++;
}

//isDigit函数检查当前位置的字符是否为数字。
bool Lexer::isDigit() const{
    if(peek()<'0'||peek()>'9') return false;
    return true;
}

//scanNumber函数负责扫描一个数字，并返回一个Token对象，表示这个数字的值。
Token Lexer::scanNumber() {
    double val=advance()-'0';
    while(isDigit()){
        val*=10;
        val+=(advance()-'0');
    }
    return {TokenType::Number, val, "val"};
}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), pos_(0) {
}

//parse函数是语法分析的入口，负责从Token序列中解析出一个表达式，并返回其计算结果。
double Parser::parse() {
    double result=parseExpression();
    consume(TokenType::End,"expected end");
    return result;
}

//current函数返回当前的token。
const Token& Parser::current() const {
    return tokens_[pos_];
}

//check函数检查当前token是否具有指定的类型。
bool Parser::check(TokenType type) const {
    if(current().type==type) return true;
    return false;
}

//match函数如果当前token具有指定的类型，则消费该token并返回true。
bool Parser::match(TokenType type) {
    if(check(type)){
        pos_++;
        return true;
    }
    return false;
}

//consume函数在当前token与指定类型匹配时消费并返回它，否则抛出异常。
const Token& Parser::consume(TokenType type, const std::string& message) {
    if(check(type)){
        pos_++;
        return tokens_[pos_-1];
    }
    throw std::runtime_error(message);
}

double Parser::parseExpression() {
    // E -> T E'
    double left=parseTerm();
    return parseExpressionPrime(left);
}

//parseExpressionPrime函数处理表达式中的加法和减法操作，接受一个继承值（inheritedValue），
//表示已经解析的部分表达式的值。
double Parser::parseExpressionPrime(double inheritedValue) {
    // E' -> + T E' | - T E' | epsilon
    //错误写法对照：先parseTerm()再match运算符，会在当前token是+或-时报错。
    //double right=parseTerm();
    //if(match(TokenType::Plus)) return parseExpressionPrime(inheritedValue+right);
    //if(match(TokenType::Minus)) return parseExpressionPrime(inheritedValue-right);
    if(match(TokenType::Plus)){
        double right=parseTerm();
        return parseExpressionPrime(inheritedValue+right);
    }
    if(match(TokenType::Minus)){
        double right=parseTerm();
        return parseExpressionPrime(inheritedValue-right);
    }
    return inheritedValue;
}

double Parser::parseTerm() {
    // T -> F T'
    double left=parseFactor();
    return parseTermPrime(left);
}

double Parser::parseTermPrime(double inheritedValue) {
    // T' -> * F T' | / F T' | epsilon
    //错误写法对照：先parseFactor()再match运算符，会在当前token是*或/时报错。
    //double right=parseFactor();
    //if(match(TokenType::Multiply)) return parseTermPrime(inheritedValue*right);
    //if(match(TokenType::Divide)) return parseTermPrime(inheritedValue/right);
    if(match(TokenType::Multiply)){
        double right=parseFactor();
        return parseTermPrime(inheritedValue*right);
    }
    if(match(TokenType::Divide)){
        double right=parseFactor();
        return parseTermPrime(inheritedValue/right);
    }
    return inheritedValue;
}

double Parser::parseFactor() {
    // F -> NUM | (E)
    if(check(TokenType::Number)){
        Token number=consume(TokenType::Number,"expected number");
        return number.value;
    }
    if(match(TokenType::LeftParen)){
        double res=parseExpression();
        consume(TokenType::RightParen,"expected rightparen");
        return res;
    }
    throw std::runtime_error("illegal");
}

int main() {
    std::string line;

    std::cout << "Enter expression: ";
    std::getline(std::cin, line);

    try {
        Lexer lexer(line);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        double result = parser.parse();

        std::cout << "Result: " << result << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
