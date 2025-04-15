// === ArcOut Expanded Compiler+VM in C++ ===

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <string>
#include <tuple>
#include <cstdint>

using namespace std;

// === Helper Functions ===
string readFile(const string& filename) {
    ifstream file(filename);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void saveBinary(const vector<vector<int>>& bytecode, const string& filename) {
    ofstream file(filename, ios::binary);
    for (const auto& instr : bytecode) {
        string op;
        int op_code = instr[0];
        switch (op_code) {
            case 1: op = "SET"; break;
            case 2: op = "ADD"; break;
            case 3: op = "SUB"; break;
            case 4: op = "MUL"; break;
            case 5: op = "DIV"; break;
            case 6: op = "MOD"; break;
            case 7: op = "CMP"; break;
            case 8: op = "JMP_IF_FALSE"; break;
            case 9: op = "JMP"; break;
            case 10: op = "RET"; break;
        }
        uint8_t len = op.size();
        file.write(reinterpret_cast<const char*>(&len), 1);
        file.write(op.c_str(), len);
        for (size_t i = 1; i < instr.size(); ++i) {
            file.write(reinterpret_cast<const char*>(&instr[i]), sizeof(int));
        }
    }
}

// === Lexer ===
class ArcLexer {
public:
    vector<vector<string>> tokenize(const string& code) {
        vector<vector<string>> tokens;
        istringstream stream(code);
        string line;
        while (getline(stream, line)) {
            smatch match;
            if (regex_match(line, match, regex(R"(let\s+(\w+)\s*=\s*(-?\d+))")))
                tokens.push_back({"SET", match[1], match[2]});
            else if (regex_match(line, match, regex(R"((\w+)\s*=\s*(\w+)\s*\+\s*(-?\w+))")))
                tokens.push_back({"ADD", match[1], match[2], match[3]});
            else if (regex_match(line, match, regex(R"((\w+)\s*=\s*(\w+)\s*-\s*(-?\w+))")))
                tokens.push_back({"SUB", match[1], match[2], match[3]});
            else if (regex_match(line, match, regex(R"((\w+)\s*=\s*(\w+)\s*\*\s*(-?\w+))")))
                tokens.push_back({"MUL", match[1], match[2], match[3]});
            else if (regex_match(line, match, regex(R"((\w+)\s*=\s*(\w+)\s*/\s*(-?\w+))")))
                tokens.push_back({"DIV", match[1], match[2], match[3]});
            else if (regex_match(line, match, regex(R"(return\s+(\w+))")))
                tokens.push_back({"RET", match[1]});
        }
        return tokens;
    }
};

// === Compiler ===
class ArcCompiler {
public:
    vector<vector<int>> bytecode;
    unordered_map<string, int> variables;

    vector<vector<int>> compile(const vector<vector<string>>& tokens) {
        for (const auto& token : tokens) {
            const string& op = token[0];
            if (op == "SET") {
                int var_id = resolve(token[1]);
                int val = resolve(token[2]);
                emit("SET", var_id, val);
            } else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV") {
                int a = resolve(token[1]);
                int b = resolve(token[2]);
                int c = resolve(token[3]);
                emit(op, a, b, c);
            } else if (op == "RET") {
                int a = resolve(token[1]);
                emit("RET", a);
            }
        }
        return bytecode;
    }

private:
    void emit(const string& op, int a = -1, int b = -1, int c = -1) {
        bytecode.push_back({hashOp(op), a, b, c});
    }

    int resolve(const string& val) {
        if (regex_match(val, regex("-?\\d+"))) {
            return stoi(val);
        }
        if (variables.find(val) == variables.end()) {
            int id = variables.size();
            variables[val] = id;
        }
        return variables[val];
    }

    int hashOp(const string& op) {
        static unordered_map<string, int> opcodes = {
            {"SET", 1}, {"ADD", 2}, {"SUB", 3}, {"MUL", 4},
            {"DIV", 5}, {"MOD", 6}, {"CMP", 7}, {"JMP_IF_FALSE", 8},
            {"JMP", 9}, {"RET", 10}
        };
        return opcodes[op];
    }
};

// === Runtime VM === (Unchanged from your original)
class ArcVM {
    vector<int> vars;
    bool cmp_result;

public:
    ArcVM() : vars(256, 0), cmp_result(false) {}

    void run(const string& filename) {
        ifstream file(filename, ios::binary);
        vector<vector<int>> code;

        while (file.peek() != EOF) {
            uint8_t len;
            file.read(reinterpret_cast<char*>(&len), 1);
            string op(len, '\0');
            file.read(&op[0], len);
            vector<int> instr = {hashOp(op)};
            int val;
            while (file.read(reinterpret_cast<char*>(&val), sizeof(int)))
                instr.push_back(val);
            file.clear();
            file.seekg(sizeof(int) * (instr.size() - 1), ios::cur);
            code.push_back(instr);
        }

        int pc = 0;
        while (pc < code.size()) {
            const auto& instr = code[pc];
            int op = instr[0];
            switch (op) {
                case 1: vars[instr[1]] = instr[2]; break;
                case 2: vars[instr[1]] = vars[instr[2]] + instr[3]; break;
                case 3: vars[instr[1]] = vars[instr[2]] - instr[3]; break;
                case 4: vars[instr[1]] = vars[instr[2]] * instr[3]; break;
                case 5: vars[instr[1]] = vars[instr[2]] / instr[3]; break;
                case 6: vars[instr[1]] = vars[instr[2]] % instr[3]; break;
                case 7: cmp_result = (vars[instr[1]] == instr[2]); break;
                case 8: if (!cmp_result) { pc = instr[1]; continue; } break;
                case 9: pc = instr[1]; continue;
                case 10: cout << "Return: " << vars[instr[1]] << endl; return;
            }
            ++pc;
        }
    }

    int hashOp(const string& op) {
        static unordered_map<string, int> opcodes = {
            {"SET", 1}, {"ADD", 2}, {"SUB", 3}, {"MUL", 4},
            {"DIV", 5}, {"MOD", 6}, {"CMP", 7}, {"JMP_IF_FALSE", 8},
            {"JMP", 9}, {"RET", 10}
        };
        return opcodes[op];
    }
};

// === Entry Point ===
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: arcbuild <source.arc> | arcrun <binary.arx>\n";
        return 1;
    }

    string mode = argv[1];
    string filename = argv[2];

    if (mode == "arcbuild") {
        ArcLexer lexer;
        string source = readFile(filename);
        auto tokens = lexer.tokenize(source);
        ArcCompiler compiler;
        auto bytecode = compiler.compile(tokens);
        saveBinary(bytecode, "main.arx");
        cout << "Compiled successfully to main.arx\n";
    } else if (mode == "arcrun") {
        ArcVM vm;
        vm.run(filename);
    }

    return 0;
}
