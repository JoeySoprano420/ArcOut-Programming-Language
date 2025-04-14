# === ArcOut Expanded Compiler+VM with While, Arithmetic, and Complex Branching ===

import sys
import re
import struct
import os

# === Lexer ===
class ArcLexer:
    def tokenize(self, code: str):
        token_spec = [
            ('FUNC', r'func\s+(\w+)'),
            ('LET', r'let\s+(\w+)\s*=\s*(-?\d+)'),
            ('RETURN', r'return\s+(\w+)'),
            ('IF', r'if\s+(\w+)\s*([><=!]+)\s*(\w+|\d+)'),
            ('WHILE', r'while\s+(\w+)\s*([><=!]+)\s*(\w+|\d+)'),
            ('ADD', r'(\w+)\s*=\s*(\w+)\s*\+\s*(\w+|\d+)'),
            ('SUB', r'(\w+)\s*=\s*(\w+)\s*-\s*(\w+|\d+)'),
            ('MUL', r'(\w+)\s*=\s*(\w+)\s*\*\s*(\w+|\d+)'),
            ('DIV', r'(\w+)\s*=\s*(\w+)\s*/\s*(\w+|\d+)'),
            ('MOD', r'(\w+)\s*=\s*(\w+)\s*%\s*(\w+|\d+)'),
            ('ELSE', r'else'),
            ('LBRACE', r'\{'),
            ('RBRACE', r'\}'),
            ('SKIP', r'[ \t\n]+'),
        ]

        tok_regex = '|'.join(f'(?P<{name}>{pattern})' for name, pattern in token_spec)
        tokens = []
        for mo in re.finditer(tok_regex, code):
            kind = mo.lastgroup
            value = mo.group()
            if kind == 'SKIP':
                continue
            if kind in {'FUNC', 'RETURN', 'ELSE', 'LBRACE', 'RBRACE'}:
                tokens.append((kind, value))
            elif kind == 'LET':
                var, val = re.match(r'let\s+(\w+)\s*=\s*(-?\d+)', value).groups()
                tokens.append(('LET', var, int(val)))
            elif kind in {'ADD', 'SUB', 'MUL', 'DIV', 'MOD'}:
                target, op1, op2 = re.match(r'(\w+)\s*=\s*(\w+)\s*[+\-*/%]\s*(\w+|\d+)', value).groups()
                tokens.append((kind, target, op1, op2))
            elif kind == 'IF':
                var1, op, var2 = re.match(r'if\s+(\w+)\s*([><=!]+)\s*(\w+|\d+)', value).groups()
                tokens.append(('IF', var1, op, var2))
            elif kind == 'WHILE':
                var1, op, var2 = re.match(r'while\s+(\w+)\s*([><=!]+)\s*(\w+|\d+)', value).groups()
                tokens.append(('WHILE', var1, op, var2))
        return tokens

# === Parser & Compiler ===
class ArcCompiler:
    def __init__(self):
        self.bytecode = []
        self.labels = {}
        self.patch_locations = []
        self.variables = {}
        self.addr = 0

    def compile(self, tokens):
        idx = 0
        while idx < len(tokens):
            token = tokens[idx]
            kind = token[0]

            if kind == 'LET':
                _, var, val = token
                self.variables[var] = len(self.variables)
                self.emit('SET', self.variables[var], val)

            elif kind in {'ADD', 'SUB', 'MUL', 'DIV', 'MOD'}:
                op_map = {'ADD': 'ADD', 'SUB': 'SUB', 'MUL': 'MUL', 'DIV': 'DIV', 'MOD': 'MOD'}
                _, target, op1, op2 = token
                self.emit(op_map[kind], self._resolve(target), self._resolve(op1), self._resolve(op2))

            elif kind == 'IF':
                _, var1, op, var2 = token
                self.emit('CMP', self._resolve(var1), self._resolve(var2))
                self.emit('JMP_IF_FALSE', 0)
                self.patch_locations.append((len(self.bytecode)-1, 'IF'))

            elif kind == 'WHILE':
                start = len(self.bytecode)
                _, var1, op, var2 = token
                self.emit('CMP', self._resolve(var1), self._resolve(var2))
                self.emit('JMP_IF_FALSE', 0)
                self.patch_locations.append((len(self.bytecode)-1, 'WHILE', start))

            elif kind == 'RETURN':
                _, var = token
                self.emit('RET', self._resolve(var))

            elif kind == 'LBRACE':
                pass

            elif kind == 'RBRACE':
                if self.patch_locations:
                    last_patch = self.patch_locations.pop()
                    if last_patch[1] == 'WHILE':
                        end = len(self.bytecode)
                        self.emit('JMP', last_patch[2])
                        self.bytecode[last_patch[0]][1] = end
                    elif last_patch[1] == 'IF':
                        self.bytecode[last_patch[0]][1] = len(self.bytecode)
            idx += 1

        return self.bytecode

    def emit(self, *args):
        self.bytecode.append(list(args))

    def _resolve(self, val):
        if isinstance(val, int) or (isinstance(val, str) and val.lstrip('-').isdigit()):
            return int(val)
        if val not in self.variables:
            self.variables[val] = len(self.variables)
        return self.variables[val]

# === Binary Linker ===
def save_binary(bytecode, filename):
    with open(filename, 'wb') as f:
        for instr in bytecode:
            opcode = instr[0].encode('utf-8')
            f.write(struct.pack('B', len(opcode)))
            f.write(opcode)
            for arg in instr[1:]:
                f.write(struct.pack('i', arg))

# === Runtime VM ===
class ArcVM:
    def __init__(self):
        self.vars = [0] * 256

    def run(self, filename):
        code = []
        with open(filename, 'rb') as f:
            while chunk := f.read(1):
                oplen = struct.unpack('B', chunk)[0]
                op = f.read(oplen).decode('utf-8')
                args = []
                while True:
                    pos = f.tell()
                    intbuf = f.read(4)
                    if not intbuf:
                        break
                    try:
                        args.append(struct.unpack('i', intbuf)[0])
                    except:
                        f.seek(pos)
                        break
                code.append((op, *args))

        pc = 0
        while pc < len(code):
            instr = code[pc]
            op = instr[0]
            args = instr[1:]
            if op == 'SET':
                self.vars[args[0]] = args[1]
            elif op == 'ADD':
                self.vars[args[0]] = self.vars[args[1]] + self._val(args[2])
            elif op == 'SUB':
                self.vars[args[0]] = self.vars[args[1]] - self._val(args[2])
            elif op == 'MUL':
                self.vars[args[0]] = self.vars[args[1]] * self._val(args[2])
            elif op == 'DIV':
                self.vars[args[0]] = self.vars[args[1]] // self._val(args[2])
            elif op == 'MOD':
                self.vars[args[0]] = self.vars[args[1]] % self._val(args[2])
            elif op == 'CMP':
                self._cmp_result = self.vars[args[0]] == self._val(args[1])
            elif op == 'JMP_IF_FALSE':
                if not self._cmp_result:
                    pc = args[0]
                    continue
            elif op == 'JMP':
                pc = args[0]
                continue
            elif op == 'RET':
                print("Return:", self.vars[args[0]])
                break
            pc += 1

    def _val(self, arg):
        return arg if isinstance(arg, int) else self.vars[arg]

# === CLI Interface ===
def arcbuild(source_file):
    with open(source_file) as f:
        code = f.read()
    lexer = ArcLexer()
    tokens = lexer.tokenize(code)
    compiler = ArcCompiler()
    bytecode = compiler.compile(tokens)
    save_binary(bytecode, 'main.arx')
    print("Compiled to main.arx")

def arcrun(binary_file):
    vm = ArcVM()
    vm.run(binary_file)

# === Command Dispatch ===
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: arcbuild <source.arc> | arcrun <binary.arx>")
    elif sys.argv[1] == 'arcbuild':
        arcbuild(sys.argv[2])
    elif sys.argv[1] == 'arcrun':
        arcrun(sys.argv[2])
