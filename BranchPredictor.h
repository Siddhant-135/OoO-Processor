#pragma once
#include "Basics.h"
#include <iostream>
#include <vector>

struct BP_info {
    bool valid = false;
    bool is_conditional = false;
    bool is_jump = false;
    int fetch_pc = -1;
    int target_pc = -1;
    int fallthrough_pc = -1;
    int predicted_next_pc = -1;
};

class BranchPredictor {
private:
    void expand_if_required(int pc) ;
    bool is_conditional_branch(OpCode op) ;

public:
    int total_branches = 0;
    int correct_predictions = 0;
    std::vector<int> state_table; // 2-bit state: State 0- Strong taken. State 1- Weak taken. State 2- Weak not taken. State 3- Strong not taken.

    int predict(int current_pc, int target_pc, OpCode op);
    BP_info make_prediction_info(int current_pc, int target_pc, OpCode op);
    void update(int pc, bool taken, bool was_correct);
    };
