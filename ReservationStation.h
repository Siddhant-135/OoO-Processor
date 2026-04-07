#pragma once
#include "Basics.h"
#include <vector>
#include <optional>

class RS {
    public:
    //constructor
    RS(int size){
        this->size=size;
        RS_stage_vector.resize(size);
    }

    bool isFull();

    //push to RS
    void push(RSEntry temp);//an O(n) search.

    //method to iterate linearly and find the ready entry: optional result?
    std::optional<RSEntry> get_valid_entry();

    //method to remove the evaluated entry from RS: rather, just update the RS. Returns a nullptr if there is no ready entry yet. 
    std::optional<RSEntry> get_ready_entry();
    //every cycle of execute, check if there is an instruction whose cycle count is pipeline length
    //if so, fetch it and do the math. The exe unit must then broadcast the result. Also delete it?

    private:
    //or store pair: RS entry, pipline stage in a maxheap: push, pop etc sort? no not just the topmost, but otehr pipelie entryes need updation too. 
    std::vector <std::pair<bool, std::pair<RSEntry, int>>> RS_stage_vector;
    int size;
};