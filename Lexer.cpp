#include <regex>
#include <string>
#include <vector>
#include <tuple>

enum class TokenType {
    FUNC, LET, RETURN, IF, WHILE,
    ADD, SUB, MUL, DIV, MOD,
    ELSE, LBRACE, RBRACE, SKIP
};

struct Token {
    TokenType type;
    std::vector<std::string> values;
};

class ArcLexer {
public:
    std::vector<Token> tokenize(const std::string& code) {
        std::vector<Token> tokens;
        std::vector<std::pair<TokenType, std::string>> token_spec = {
            {TokenType::FUNC, R"(func\s+(\w+))"},
            {TokenType::LET, R"(let\s+(\w+)\s*=\s*(-?\d+))"},
            {TokenType::RETURN, R"(return\s+(\w+))"},
            {TokenType::IF, R"(if\s+(\w+)\s*([><=!]+)\s*(\w+|\d+))"},
            {TokenType::WHILE, R"(while\s+(\w+)\s*([><=!]+)\s*(\w+|\d+))"},
            {TokenType::ADD, R"((\w+)\s*=\s*(\w+)\s*\+\s*(\w+|\d+))"},
            {TokenType::SUB, R"((\w+)\s*=\s*(\w+)\s*-\s*(\w+|\d+))"},
            {TokenType::MUL, R"((\w+)\s*=\s*(\w+)\s*\*\s*(\w+|\d+))"},
            {TokenType::DIV, R"((\w+)\s*=\s*(\w+)\s*/\s*(\w+|\d+))"},
            {TokenType::MOD, R"((\w+)\s*=\s*(\w+)\s*%\s*(\w+|\d+))"},
            {TokenType::ELSE, R"(else)"},
            {TokenType::LBRACE, R"(\{)"},
            {TokenType::RBRACE, R"(\})"},
            {TokenType::SKIP, R"([ \t\n]+)"}
        };

        std::string remaining = code;
        while (!remaining.empty()) {
            bool matched = false;
            for (const auto& [type, pattern] : token_spec) {
                std::regex re("^" + pattern);
                std::smatch match;
                if (std::regex_search(remaining, match, re)) {
                    if (type != TokenType::SKIP) {
                        Token token;
                        token.type = type;
                        for (size_t i = 1; i < match.size(); ++i) {
                            token.values.push_back(match[i]);
                        }
                        tokens.push_back(token);
                    }
                    remaining = match.suffix().str();
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                throw std::runtime_error("Unknown token in input");
            }
        }
        return tokens;
    }
};

