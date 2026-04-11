#include "RegisterAliasTable.h"

void RAT::add_to_RAT (int idx, int ROBId){
    RAT_vector[idx].valid = false;
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