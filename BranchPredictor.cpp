#include "Basics.h"
#include "BranchPredictor.h"

void BranchPredictor::expand_if_required(int pc) {
    if (pc < 0) return;
    if (pc >= static_cast<int>(state_table.size())) state_table.resize(pc + 1, 0); // need the static cast for unsigned values vaise to, which we already remove in part 1. but letting it stay given kitni dikkatein anyways hai hi iss assignment mai
}

bool BranchPredictor::is_conditional_branch(OpCode op) {return (op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE);}

int BranchPredictor::predict(int current_pc, int target_pc, OpCode op) {
    if (op == OpCode::J) return target_pc;

    if (is_conditional_branch(op)) {
        expand_if_required(current_pc);
        int state = state_table[current_pc];
        if (state <= 1) return target_pc;
        return (current_pc + 1);
    }
    return current_pc + 1;
}

BP_info BranchPredictor::make_prediction_info(int current_pc, int target_pc, OpCode op) { 
    BP_info info;
    info.valid = true;
    info.fetch_pc = current_pc;
    info.target_pc = target_pc;
    info.fallthrough_pc = current_pc + 1;
    info.is_conditional = is_conditional_branch(op);
    info.is_jump = (op == OpCode::J);
    info.predicted_next_pc = predict(current_pc, target_pc, op);
    return info;
}

void BranchPredictor::update(int pc, bool taken, bool was_correct) {
    expand_if_required(pc);
    total_branches++;
    if (was_correct) correct_predictions++;
    int& state = state_table[pc];
    if(taken) state = std::max(0, state - 1); 
    else state = std::min(3, state + 1); 
    }
