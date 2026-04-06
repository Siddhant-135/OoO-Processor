#include "Parser.h"
#include <optional>

std::optional<OpCode> Parser::parseOperation(std::string_view first_token)
{
    if (first_token == "add") return OpCode::ADD;
    else if (first_token == "sub") return OpCode::SUB;
    else if (first_token == "addi") return OpCode::ADDI;
    else if (first_token == "mul") return OpCode::MUL;
    else if (first_token == "div") return OpCode::DIV;
    else if (first_token == "rem") return OpCode::REM;
    else if (first_token == "lw") return OpCode::LW;
    else if (first_token == "sw") return OpCode::SW;
    else if (first_token == "beq") return OpCode::BEQ;
    else if (first_token == "bne") return OpCode::BNE;
    else if (first_token == "blt") return OpCode::BLT;
    else if (first_token == "ble") return OpCode::BLE;
    else if (first_token == "j") return OpCode::J;
    else if (first_token == "slt") return OpCode::SLT;
    else if (first_token == "slti") return OpCode::SLTI;
    else if (first_token == "and") return OpCode::AND;
    else if (first_token == "or") return OpCode::OR;
    else if (first_token == "xor") return OpCode::XOR;
    else if (first_token == "andi") return OpCode::ANDI;
    else if (first_token == "ori") return OpCode::ORI;
    else if (first_token == "xori") return OpCode::XORI;
    else return std::nullopt;
};


Instruction Parser::parseFile(std::ifstream& file, std::vector<Instruction>& inst_memory)
{
    std::string line;
    while (std::getline(file, line)) 
    {
        std::istringstream iss(line); // could be a tag, or a operation, or a comment hash, or a memory declaration
        std::string first_word;
        iss >> first_word;

        if (parseOperation(first_word).has_value())
        {
            OpCode op = parseOperation(first_word).value();
            Instruction inst;
            inst.op = op;
            if (op==ADD )
        }
        else if (first_word[0]=='.' && first_word[first_word.length() - 1]==':') 
        {
            break;
        }
        else if (first_word[first_word.length() - 1]==':')
        {
            break;
        }
        else{
            throw std::runtime_error("File is corrupted in its input");
            return;
        }
    }
}
    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#');
        Instruction inst = lineToInst (line);
        inst_memory.push_back(inst);
    }