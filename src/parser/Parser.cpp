#include "Parser.h"

std::optional<OpCode> Parser::parseOperation(std::string_view first_token)
{
    if (first_token == "add") return OpCode::ADD;// done
    else if (first_token == "sub") return OpCode::SUB;// done
    else if (first_token == "addi") return OpCode::ADDI;// done
    else if (first_token == "mul") return OpCode::MUL;// done
    else if (first_token == "div") return OpCode::DIV;// done
    else if (first_token == "rem") return OpCode::REM;// done
    else if (first_token == "lw") return OpCode::LW; // done
    else if (first_token == "sw") return OpCode::SW; // done
    else if (first_token == "beq") return OpCode::BEQ; // done
    else if (first_token == "bne") return OpCode::BNE; // done
    else if (first_token == "blt") return OpCode::BLT; // done
    else if (first_token == "ble") return OpCode::BLE; // done
    else if (first_token == "j") return OpCode::J; // done
    else if (first_token == "slt") return OpCode::SLT; // done
    else if (first_token == "slti") return OpCode::SLTI; // done
    else if (first_token == "and") return OpCode::AND; // done
    else if (first_token == "or") return OpCode::OR; // done
    else if (first_token == "xor") return OpCode::XOR; // done
    else if (first_token == "andi") return OpCode::ANDI; // done
    else if (first_token == "ori") return OpCode::ORI; // done
    else if (first_token == "xori") return OpCode::XORI; // done
    else return std::nullopt;
};

int Parser::forward_pass_fixup(const std::string& label, int inst_number, std::vector<std::pair<int, std::string>>& pending_label_fixups)
{
    int target = -1;
    for (const auto& p : inst_alias) {
        if (p.first == label) {
            target = p.second;
            break;
        }
    }
    if (target == -1) {
        pending_label_fixups.push_back({inst_number, label});
        return 0;
    }
    return target;
}

void Parser::backward_pass_fixup(std::vector<Instruction>& inst_memory, const std::vector<std::pair<int, std::string>>& pending_label_fixups)
{
    for (const auto& fixup : pending_label_fixups) {
        int inst_idx = fixup.first;
        const std::string& label = fixup.second;
        inst_memory[inst_idx].imm = Parser::getValue(inst_alias, label);
    }
}

void Parser::parseFile(std::ifstream& file, std::vector<Instruction>& inst_memory, std::vector<int>& memory)
{
    inst_alias.clear();
    mem_alias.clear();
    std::string line;
    int inst_number = 0;
    int mem_location = 0;
    std::vector<std::pair<int, std::string>> pending_label_fixups;
    while (std::getline(file, line)) 
    {
        std::istringstream iss(line); // 4 options: could be a tag, or a operation, or a comment hash, or a memory declaration
        std::string first_word;
        iss >> first_word;
// ARITHMETIC OPERATIONS, BRANCHES, JUMPS == pushback inst to inst_memory 
        if(first_word.empty() || first_word[0]=='#') continue; // comment line, ignore
        else if (parseOperation(first_word).has_value())
        {
            OpCode op = parseOperation(first_word).value();
            Instruction inst;
            inst.op = op;
            inst.pc = inst_number;

            if(op==OpCode::LW) // Instruction type lw x5 A(x4)  to be interpreted as src1 = x4, imm = A, dest = x5. sw is different and separately handled in the sw case below. 
            {
                std::string dest, source;
                iss >> dest >> source;
                int dest_reg = std::stoi(dest.substr(1)); // works for double digit as well. note that dest[1] wouldn't work coz thats a pointer
                inst.dest = dest_reg;
                auto l = source.find('(');
                auto r = source.find(')');
                std::string mem_loc = source.substr(0, l);
                std::string reg = source.substr(l+1, r-l-1); // syntax is start, length not start, end.
                inst.src1 = std::stoi(reg.substr(1));
                inst.imm = Parser::getValue(mem_alias, mem_loc); // Implementation: store memory[imm + value from src1] into dest.
            }
            else if (op==OpCode::SW) // Instruction type sw x5 A(x4)  to be interpreted as src1 = x5, imm = A, src2 = x4. lw is different and separately handled in the lw case above.
            {
                std::string source, src2;
                iss >> source >> src2;
                int src_reg = std::stoi(source.substr(1)); // just the reg number. the vakue is accessed by the Processor ka ARF, we dont that access to it here.
                inst.src1 = src_reg; // value from src1 will be stored into memory[imm + value from dest]
                auto l = src2.find('(');
                auto r = src2.find(')');
                std::string mem_loc = src2.substr(0, l); //the memory array that it is part of.
                std::string reg = src2.substr(l+1, r-l-1);
                inst.src2 = std::stoi(reg.substr(1));
                inst.imm = Parser::getValue(mem_alias, mem_loc);
            }
            else if (op==OpCode::BEQ || op==OpCode::BNE || op==OpCode::BLT || op==OpCode::BLE)
            {
                std::string src1, src2, label;
                iss >> src1 >> src2 >> label;
                inst.src1 = std::stoi(src1.substr(1));
                inst.src2 = std::stoi(src2.substr(1));
                inst.imm = forward_pass_fixup(label, inst_number, pending_label_fixups);
            }
            else if (op==OpCode::J)
            {
                std::string label;
                iss >> label;
                inst.imm = forward_pass_fixup(label, inst_number, pending_label_fixups);
            }
            else if (op==OpCode::ADDI || op==OpCode::SLTI || op==OpCode::ANDI || op==OpCode::ORI || op==OpCode::XORI)
            {
                std::string dest, source, imm;
                iss >> dest >> source >> imm;
                int dest_reg = std::stoi(dest.substr(1)); 
                inst.dest = dest_reg;
                inst.src1 = std::stoi(source.substr(1));
                inst.imm = std::stoi(imm); // Implementation: perform the operation on value from src1 and imm, store into dest.
            }
            else // ADD, SUB, MUL, DIV, REM, SLT, AND, OR, XOR of the form add x5 x3 x4 to be interpreted as src1 = x3, src2 = x4, dest = x5.
            {
                std::string dest, src1, src2;
                iss >> dest >> src1 >> src2;
                int dest_reg = std::stoi(dest.substr(1)); 
                inst.dest = dest_reg;
                inst.src1 = std::stoi(src1.substr(1));
                inst.src2 = std::stoi(src2.substr(1)); // Implementation: perform the operation on value from src1 and value from src2, store into dest.
            }
            inst_memory.push_back(inst);
            inst_number++;
        }
// MEMORY FILLING AND LABELS
        else if (first_word[0]=='.' && first_word[first_word.length() - 1]==':') // Memory declaration of the form .A: 1 2 4 6, to be interpreted as wherever A starts, there is 1, then 2 then 4. Location of A itself is determined by addition assuming all memory contiguous.
        {
            std::string mem_name = first_word.substr(1, first_word.length()-2); // remove the dot and the colon 
            mem_alias.push_back({mem_name, mem_location});
            int val;
            while (iss >> val)
            {
                memory[mem_location] = val;
                mem_location++;
            }
            continue;
        }
// PC LABEL DECLARATIONS == push label, pointed pc into instruction alias 
        else if (first_word[first_word.length() - 1]==':')
        {
            std::string label_name = first_word.substr(0, first_word.length()-1);
            inst_alias.push_back({label_name, inst_number}); // label points to the current next instruction index in stream.
            continue;
        }

        else{
            throw std::runtime_error("File is corrupted in its input");
        }
    }

    backward_pass_fixup(inst_memory, pending_label_fixups);
}
// FINDS location at which a label or memory tag is stored in the actual alias table.
int Parser::getValue(const std::vector<std::pair<std::string, int>>& aliastable, std::string_view name)
{
    for (auto& pair : aliastable)
    {
        if (pair.first == name) return pair.second;
    }
    throw std::runtime_error("Label not found"); // not found
}
