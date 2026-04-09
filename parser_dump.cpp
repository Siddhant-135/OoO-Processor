#include <fstream>
#include <iostream>
#include <vector>
#include "Parser.h"

static const char* opToString(OpCode op) {
    switch (op) {
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::ADDI: return "ADDI";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::REM: return "REM";
        case OpCode::LW: return "LW";
        case OpCode::SW: return "SW";
        case OpCode::BEQ: return "BEQ";
        case OpCode::BNE: return "BNE";
        case OpCode::BLT: return "BLT";
        case OpCode::BLE: return "BLE";
        case OpCode::J: return "J";
        case OpCode::SLT: return "SLT";
        case OpCode::SLTI: return "SLTI";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::XOR: return "XOR";
        case OpCode::ANDI: return "ANDI";
        case OpCode::ORI: return "ORI";
        case OpCode::XORI: return "XORI";
    }
    return "UNKNOWN";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./parser_dump <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    Parser parser;
    std::vector<Instruction> inst_memory;
    std::vector<int> memory;

    try {
        parser.parseFile(file, inst_memory, memory);
    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Parsed " << inst_memory.size() << " instructions:\n";
    for (int i = 0; i < static_cast<int>(inst_memory.size()); i++) {
        const Instruction& inst = inst_memory[i];
        std::cout
            << i
            << ": op=" << opToString(inst.op)
            << " dest=" << inst.dest
            << " src1=" << inst.src1
            << " src2=" << inst.src2
            << " imm=" << inst.imm
            << " pc=" << inst.pc
            << "\n";
    }

    return 0;
}
