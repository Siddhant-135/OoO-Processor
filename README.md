# Assignment 2: Out-of-Order Execution

Having seen the RISC-V ISA and pipelined processor implementation, we now move to improving the efficiency of processors by allowing instructions to execute *out of program order*. A common way to do this is by using **Reservation Stations**, **Register Alias Table** and multiple **execution units**.

However, allowing this makes it difficult to deal with exceptions, such as when an instruction causes division by zero. In such a scenario, we want to be able to preserve the architectural state until right before the exception-causing instruction, and then report the instruction which caused the exception. 

So, the architectural state during program execution should be modified strictly in program order. The way to support this is to use **Reorder Buffers**.

## Problem Statement

Implement the (32-bit) out-of-order processor with precise exceptions, utilizing a register alias table, per-execution-unit reservation stations and a reorder buffer. 

You can also implement a compiler which translates the input RISC-V file to a format that is easier for your C++ code to read.

The provided starter code will give you the framework within which to work. Feel free to add members to each class, but do not remove any existing ones.

The number of registers, reorder buffer size, reservation station entries per unit, execution latencies and memory size will be passed as a configurable input to the Processor class.

The program counter, register file and memory must be updated in order and in strict compliance with the given semantics as these will be used to autograde your submission. Some testcases will be released in order to help you cross-check your implementation.

## RISC-V ISA
We will pretend that the instructions are executing on a 32-bit machine. For this reason, none of the code given to you will have `immediate` values that do not fit in 32 bits. Furthermore, exceptions will be designed based around this assumption. We will also introduce exceptions not present in typical RISC-V hardware in order to test the correctness of your precise exception handling.

You need to support the following instructions/syntax:

1. Labelled memory allocation of the form
    ```
    .A: 1 2 3 4 
    ```

2. Memory instructions: `lw`, `sw` 

    (These execute on the **LoadStoreQueue** unit)
    ```
    lw x4, B(x1)
    sw x5, C(x1)
    ```
    These will involve interacting with `Memory[i]`. If an attempt is made to access an address outside `Memory`, raise an exception.

    More details on this follow in the architectural description.


    Note that unlike a real program, our memory is simply positions in an array, so rather than +4 to go to the next item, it would simply be +1. Memory addresses will start from 0. 

3. Mathematical operations (for each of these, if the result exceeds 2**31-1 or is lower than -2\**31, raise an exception): 

    (On **Adder** unit)
    - `add`
    - `sub`
    - `addi`: Addition with immediate

    (On **Multiplier** unit)
    - `mul`
    
    (On **Divider** unit)
    - `div`: Integer division. If second operand is 0, raise an exception
    - `rem`: Remainder after dividing. If second operand is 0, raise an exception

4. Logical operations (for all of these, __no exceptions__ are raised)

    (On **Adder** unit)
    - `slt`: Set output bit to 1 if operand 1 < operand 2
    - `slti`: Set-less-than with immediate

    (On **Logic** unit)
    - `and`
    - `or`
    - `xor`
    - `andi`
    - `ori`
    - `xori` 

5. Branch operations
    - `j`: Unconditional jump. Not dispatched to any execution unit.

    (Resolved on **Branch** unit)
    - `beq`: Branch if equal.
    - `bne`: Branch if not equal.
    - `blt`: Branch if first operand is less than the second.
    - `ble`: Branch if first operand is less than or equal to the second.

    For this purpose, your compiler must support labels which will be used the compute the `immediate` for these branch instructions.

    ```
    loop:
        addi x1, x1, 1
        beq x1, x2, loop
    ```

Finally, you should also be able to ignore blank lines and comments (which will start with a `#`). Ideally after preprocessing, each increment to the PC should correspond to moving to the next instruction.

Note that you do not have to implement any of these operations bit-wise, you can simply use built-in C++ operations to compute the result after the required number of cycles have elapsed.

## Processor Design
The C++ class has the following members that need to be maintained accurately: `pc` corresponding to the Program Counter (incremented by 1 per instruction), `ARF` (register file) and the `exception` boolean (which is set in case of an exception). You must also implement the BranchPredictor `bp`. Of course, to implement Tomasulo's algorithm correctly, you also need the Register Alias Table (`RAT`), a Reorder Buffer (`ROB`) and a Reservation Station (`RS`) for each execution unit and the LoadStoreQueue (`LSQ`).

The processor is pipelined with 4 stages: Fetch, Decode, Execute/Broadcast and Commit.

Though we will execute instructions out-of-order, the most important thing to keep in mind that all changes affecting architectural state must occur in the Commit stage, using the reorder buffer to ensure changes occur in program order. This means that all writes to memory, writes to registers, exception detection, branch resolution/flushes can only occur in the Commit stage. 

`Fetch`: Fetches the instruction at `pc`. Determines the next `pc` via prediction using the `BranchPredictor bp`.

`Decode`: Decodes the instruction. Allocates an ROB entry. Also allocates the reservation station entries and updates the register alias table, if required for the instruction. Keep in mind that if the reservation station of the target unit is full, or the ROB is full, then the instruction needs to wait. Note that _every_ instruction must have an _ROB_ entry allocated.

`Execute`: All execution units/load store queue process their inputs. Note that all execution units are _also pipelined_, which means they can be processing results for multiple instructions at a time, as long as these instructions are in different stages of the units' pipeline. At the start of each cycle, the unit checks its reservation station and starts executing the oldest entry (corresponding to the oldest instruction) that is ready to be executed.

If any instructions are completed on any units, the units broadcast the computation result at the end of that very cycle (before the start of the next cycle) to all other stations/the ROB (so that they can perform tag managing and use the result). We assume a Common Data Bus of sufficient width, so that even if multiple units finish their execution in the same cycle, they can all broadcast their results. All Reservation Station entries are immediately deallocated once they are no longer needed at this time as well. 

Finally, the `LSQ` is unique in the sense that it can only execute in **sequential order**, that is, the `LSQ` does **not** execute instructions dispatched to it out-of-order. This is because Memory addresses are not aliased like registers, and therefore cannot be handled out-of-order like registers. 

All exceptions are only detected in the final cycle of the execution of the instruction. In such a case, the execution unit/load store queue sets the `exception` flag when broadcasting the result, which is passed on to the ROB entry for the instruction. The exception propagates to the Processor's `exception` bit ONLY AT THE TIME OF RETIREMENT of the instruction in the ROB (otherwise, branch mispredictions could trigger false exceptions).

`Commit`: The commit stage retires instructions / finally transfers their effects to the architectural elements, like writes to memory/registers, checking correctness of branch prediction or raising an exception. In the case of a branch misprediction, the entire pipeline needs to be flushed and the PC is set to the correct 'next instruction' (IE, if instruction at PC x was the branch and was wrongly predicted taken, after the flush when x is in the commit stage, in the next cycle the instruction fetched should be PC x+1). In the case that the commit stage of an instruction detects an exception, it should set the PC to the PC of the instruction that generated the exception, set the `exception` boolean, flush the rest of the pipeline and halt execution (do not commit anything).

In general, execution should be halted at the end of a cycle at which the ROB is empty and no more instructions are left to be processed. This can be done by returning `false` in the `Processor::step`` function, each call to which should execute exactly one cycle of the CPU as per the given specifications. The CPU will be stepped one cycle at a time and architectural state will be checked every cycle.

## Architectural Details

### Program Counter
The `pc` variable should be maintained as specified for correctness when exceptions occur. The PC starts from 0 (on the first instruction). Empty lines, comments, memory declarations, et cetera do not count as instructions and thus the PC should not be incremented because of these. It is recommended to preprocess and load the instructions into the provided `inst_memory` vector and index into it using the PC.

### Register File
The register file is simply a vector `ARF` in the `Processor` that is indexed normally. Remember that the `x0` register in RISC-V always remains 0 regardless of writes to it. (So x0 will always be up-to-date architecturally)
Note that we will never give a register name that does not exist (eg. x100) so no need to think of exception handling for this.

### Memory
The Memory is a vector whose size will be specified via the config. It can be populated using a given source code file by declaring memory labels and their space-separated contents as mentioned in the ISA section. **Unpopulated sections of Memory should be zero'ed out at program start.** Memory will be indexed like a normal vector as well, IE lw x1 A(x1) should load the data from the memory address of A + 1, that is, the next entry in the vector from where A started.
The data of multiple memory labels declared in the code should be filled sequentially on program load. That is,
```
.A: 1 2 3
.B: 4 5 6
```
should populate `Memory[0] = 1`, `Memory[1] = 2`, `Memory[2] = 3`, `Memory[3] = 4`, `Memory[4] = 5` and `Memory[5] = 6`. Any references to A and B should be appropriately replaced with 0 or 3 while preprocessing the code. The given memory to populate will never exceed the amount of memory given in the config. 
If an instruction attemps to access an out of bounds memory address, an exception should be raised.

### Branch Predictor
The Branch Predictor is a per-instruction 2-bit saturating counter. 
State 0- Starting state. Predict taken.
State 1- Predict taken.
State 2- Predict not taken.
State 3- Predict not taken.

The predictor is updated during the commit stage of a branch instruction. These updates change the state of the branch predictor for _that particular instruction_. The transitions are:
```
State 1 -if branch was taken-> State 0
State 2 -if branch was taken-> State 1
State 3 -if branch was taken-> State 2
State 0 -if branch was not taken-> State 1
State 1 -if branch was not taken-> State 2
State 2 -if branch was not taken-> State 3
```
In any other cases the state remains the same.

## Submission & Grading Instructions

To ensure your processor can be accurately tested by the auto-grader, you must provide a `Makefile` at the root of your repository. 

We have provided a sample `main.cpp` file. This file demonstrates which members and methods of your `Processor` class the auto-grader will interface with. Ensure your class structure is compatible with this file.

### Required Makefile Targets

Your `Makefile` must support the following exact commands:

**1. Compilation**
```bash
make compile FILE=<filename.cpp>
```
- **Action:** Compiles your core processor code alongside the provided C++ file in the `FILE` argument.
- **Output:** Must produce an executable named exactly `main`.


**2. Preprocessing**
```bash
make run FILE=<filename.s>
```
- **Action:** Takes the provided RISC-V assembly text file and performs any necessary text-based preprocessing (e.g., label replacement, pseudo-instruction translation).
- **Output:** Writes the preprocessed assembly back into the original text file.


**Execution Format**

Once compiled and preprocessed, the auto-grader will execute your simulator using the following format:
```bash
./main <filename.s> [additional_args...]
```
We will pass this `filename.s` as argument to the `Processor::loadProgram()` function. You do not have to write any `main()` function or worry about any command line argument handling.
