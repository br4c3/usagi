
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <utility>

// ==========================================
// 1. Token
// ==========================================

enum class TokenType {
    LET,
    IDENTIFIER,
    NUMBER,
    ASSIGN,
    SEMICOLON,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE
};

struct Token {
    TokenType type;
    std::string value;
};

// ==========================================
// 2. Lexer
// ==========================================

class Lexer {
private:
    std::string source;
    size_t pos = 0;

public:
    explicit Lexer(const std::string& input)
        : source(input) {}

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;

        while (pos < source.size()) {
            char c = source[pos];

            // 공백 처리
            if (std::isspace(static_cast<unsigned char>(c))) {
                pos++;
                continue;
            }

            // 키워드 및 식별자 처리
            if (std::isalpha(static_cast<unsigned char>(c))) {
                std::string word;

                while (pos < source.size() &&
                       std::isalpha(static_cast<unsigned char>(source[pos]))) {
                    word += source[pos];
                    pos++;
                }

                if (word == "let") {
                    tokens.push_back({TokenType::LET, word});
                } else {
                    tokens.push_back({TokenType::IDENTIFIER, word});
                }

                continue;
            }

            // 숫자 처리
            if (std::isdigit(static_cast<unsigned char>(c))) {
                std::string number;

                while (pos < source.size() &&
                       std::isdigit(static_cast<unsigned char>(source[pos]))) {
                    number += source[pos];
                    pos++;
                }

                tokens.push_back({TokenType::NUMBER, number});
                continue;
            }

            // 연산자 및 기호 처리
            switch (c) {
                case '=':
                    tokens.push_back({TokenType::ASSIGN, "="});
                    break;

                case ';':
                    tokens.push_back({TokenType::SEMICOLON, ";"});
                    break;

                case '+':
                    tokens.push_back({TokenType::PLUS, "+"});
                    break;

                case '-':
                    tokens.push_back({TokenType::MINUS, "-"});
                    break;

                case '*':
                    tokens.push_back({TokenType::MULTIPLY, "*"});
                    break;

                case '/':
                    tokens.push_back({TokenType::DIVIDE, "/"});
                    break;

                default:
                    throw std::runtime_error(
                        std::string("Unknown character: ") + c);
            }

            pos++;
        }

        return tokens;
    }
};

// ==========================================
// 3. Token 출력 함수
// ==========================================

const char* tokenTypeName(TokenType type)
{
    switch (type) {
        case TokenType::LET:
            return "LET";

        case TokenType::IDENTIFIER:
            return "IDENTIFIER";

        case TokenType::NUMBER:
            return "NUMBER";

        case TokenType::ASSIGN:
            return "ASSIGN";

        case TokenType::SEMICOLON:
            return "SEMICOLON";

        case TokenType::PLUS:
            return "PLUS";

        case TokenType::MINUS:
            return "MINUS";

        case TokenType::MULTIPLY:
            return "MULTIPLY";

        case TokenType::DIVIDE:
            return "DIVIDE";
    }

    return "UNKNOWN";
}

// ==========================================
// 4. AST
// ==========================================

// 모든 수식 노드의 기본 클래스
struct Expr {
    virtual ~Expr() = default;
};

// 숫자 노드
struct NumberExpr : Expr {
    int value;

    explicit NumberExpr(int v)
        : value(v) {}
};

// 이항 연산 노드
struct BinaryExpr : Expr {
    TokenType op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(
        TokenType operation,
        std::unique_ptr<Expr> lhs,
        std::unique_ptr<Expr> rhs
    ) : op(operation),
        left(std::move(lhs)),
        right(std::move(rhs)) {}
};

// ==========================================
// 5. Parser
// ==========================================

class Parser {
private:
    const std::vector<Token>& tokens;
    size_t pos = 0;

    // 예상한 토큰인지 확인하고 반환
    Token consume(TokenType expected)
    {
        if (pos >= tokens.size() ||
            tokens[pos].type != expected) {
            throw std::runtime_error("Syntax error");
        }

        return tokens[pos++];
    }

    // 일치하면 다음 토큰으로 이동
    bool match(TokenType type)
    {
        if (pos < tokens.size() && tokens[pos].type == type) {
            pos++;
            return true;
        }

        return false;
    }

    // 숫자 노드 생성
    std::unique_ptr<Expr> parsePrimary()
    {
        Token token = consume(TokenType::NUMBER);

        return std::make_unique<NumberExpr>(
            std::stoi(token.value)
        );
    }

    // 곱셈과 나눗셈
    std::unique_ptr<Expr> parseTerm()
    {
        auto left = parsePrimary();

        while (pos < tokens.size()) {
            TokenType op;

            if (match(TokenType::MULTIPLY)) {
                op = TokenType::MULTIPLY;
            }
            else if (match(TokenType::DIVIDE)) {
                op = TokenType::DIVIDE;
            }
            else {
                break;
            }

            auto right = parsePrimary();

            left = std::make_unique<BinaryExpr>(
                op,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    // 덧셈과 뺄셈
    std::unique_ptr<Expr> parseExpression()
    {
        auto left = parseTerm();

        while (pos < tokens.size()) {
            TokenType op;

            if (match(TokenType::PLUS)) {
                op = TokenType::PLUS;
            }
            else if (match(TokenType::MINUS)) {
                op = TokenType::MINUS;
            }
            else {
                break;
            }

            auto right = parseTerm();

            left = std::make_unique<BinaryExpr>(
                op,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

public:
    explicit Parser(const std::vector<Token>& input)
        : tokens(input) {}

    std::unique_ptr<Expr> parse()
    {
        // 변수 선언문 분석
        consume(TokenType::LET);
        consume(TokenType::IDENTIFIER);
        consume(TokenType::ASSIGN);

        // 수식 분석 및 AST 생성
        auto expression = parseExpression();

        consume(TokenType::SEMICOLON);

        // 남은 토큰 검사
        if (pos != tokens.size()) {
            throw std::runtime_error("Unexpected token");
        }

        return expression;
    }
};

// ==========================================
// 6. AST 출력 함수
// ==========================================

void printAST(const Expr* expr)
{
    if (auto number =
            dynamic_cast<const NumberExpr*>(expr)) {
        std::cout << number->value;
        return;
    }

    if (auto binary =
            dynamic_cast<const BinaryExpr*>(expr)) {

        std::cout << "(";

        switch (binary->op) {
            case TokenType::PLUS:
                std::cout << "+ ";
                break;

            case TokenType::MINUS:
                std::cout << "- ";
                break;

            case TokenType::MULTIPLY:
                std::cout << "* ";
                break;

            case TokenType::DIVIDE:
                std::cout << "/ ";
                break;

            default:
                throw std::runtime_error("Invalid operator");
        }

        printAST(binary->left.get());

        std::cout << " ";

        printAST(binary->right.get());

        std::cout << ")";
        return;
    }

    throw std::runtime_error("Invalid AST node");
}

// ==========================================
// 7. Main
// ==========================================

int main(void)
{
    std::string source = "let x = 10 + 20 * 3;";

    try {
        // 1. Lexer
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "[Lexer]\n";

        for (const Token& token : tokens) {
            std::cout << tokenTypeName(token.type)
                      << " : " << token.value << '\n';
        }

        // 2. Parser
        Parser parser(tokens);
        auto ast = parser.parse();

        std::cout << "\n[Parser]\n";
        std::cout << "Syntax OK\n";

        // 3. AST
        std::cout << "\n[AST]\n";
        printAST(ast.get());
        std::cout << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
