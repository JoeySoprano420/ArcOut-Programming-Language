#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>

enum class OpCode {
    SET, ADD, SUB, MUL, DIV, MOD,
    CMP, JMP_IF_FALSE, JMP, RET
};

struct Instruction {
    OpCode opcode;
    std::vector<int> args;
};

class ArcCompiler {
private:
    std::vector<Instruction> bytecode;
    std::unordered_map<std::string, int> variables;
    std::vector<std::tuple<size_t, std::string, size_t>> patch_locations;

    int resolve(const std::string& val) {
        if (std::regex_match(val, std::regex(R"(-?\d+)"))) {
            return std::stoi(val);
        }
        if (variables.find(val) == variables.end()) {
            variables[val] = variables.size();
        }
        return variables[val];
    }

    void emit(OpCode opcode, const std::vector<int>& args) {
        bytecode.push_back({opcode, args});
    }

public:
    std::vector<Instruction> compile(const std::vector<Token>& tokens) {
        size_t idx = 0;
        while (idx < tokens.size()) {
            const Token& token = tokens[idx];
            switch (token.type) {
                case TokenType::LET: {
                    const std::string& var = token.values[0];
                    int val = std::stoi(token.values[1]);
                    variables[var] = variables.size();
                    emit(OpCode::SET, {variables[var], val});
                    break;
                }
                case TokenType::ADD:
                case TokenType::SUB:
                case TokenType::MUL:
                case TokenType::DIV:
                case TokenType::MOD: {
                    OpCode op;
                    switch (token.type) {
                        case TokenType::ADD: op = OpCode::ADD; break;
                        case TokenType::SUB: op = OpCode::SUB; break;
                        case TokenType::MUL: op = OpCode::MUL; break;
                        case TokenType::DIV: op = OpCode::DIV; break;
                        case TokenType::MOD: op = OpCode::MOD; break;
                        default: break;
                    }
                    int target = resolve(token.values[0]);
                    int op1 = resolve(token.values[1]);
                    int op2 = resolve(token.values[2]);
                    emit(op, {target, op1, op2});
                    break;
                }
                case TokenType::IF: {
                    int var1 = resolve(token.values[0]);
                    int var2 = resolve(token.values[2]);
                    emit(OpCode::CMP, {var1, var2});
                    emit(OpCode::JMP_IF_FALSE, {0});
                    patch_locations.emplace_back(bytecode.size() - 1, "IF", 0);
                    break;
                }
                case TokenType::WHILE: {
                    size_t start = bytecode.size();
                    int var1 = resolve(token.values[0]);
                    int var2 = resolve(token.values[2]);
                    emit(OpCode::CMP, {var1, var2});
                    emit(OpCode::JMP_IF_FALSE, {0});
                    patch_locations.emplace_back(byte
::contentReference[oaicite:0]{index=0}
 
