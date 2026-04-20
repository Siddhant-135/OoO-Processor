#include "RegisterAliasTable.h"

void RAT::add_to_RAT (int idx, int ROBId){
    RAT_vector[idx].valid = false;
    // Priting handled by RAT::print() if verbose is on
    RAT_vector[idx].ROBId = ROBId;
}
//removeEntry: return it to valid and -1
void RAT::rem_from_RAT (int idx){
    RAT_vector[idx].valid = true;
    RAT_vector[idx].ROBId = -1;
}
//return the ROB entry corresponding to the index.
int RAT::get_alias (int idx){
    return RAT_vector[idx].ROBId;
}
//return whether the register is in a valid state at that time.
bool RAT::reg_valid (int idx){
    return RAT_vector[idx].valid;
}

void RAT::reset() {
    for (size_t i = 0; i < RAT_vector.size(); i++) {
        RAT_vector[i].valid = true;
        RAT_vector[i].ROBId = -1;
    }
}

void RAT::print() const {
    std::cout << "--- RAT ---\n";
    for (size_t i = 0; i < RAT_vector.size(); i++) {
        if (!RAT_vector[i].valid) {
            std::cout << " x" << i << " -> ROB[" << RAT_vector[i].ROBId << "] | ";
            if ((i + 1) % 4 == 0) std::cout << "\n";
        }
    }
    std::cout << "\n";
}
