# Out-of-Order RISC-V Processor Simulator

This repository contains my C++ implementation of a 32-bit out-of-order RISC-V-style processor with precise exceptions. The design combines Tomasulo-style scheduling with a **Register Alias Table (RAT)**, **per-unit reservation stations**, a **Reorder Buffer (ROB)**, a **Load/Store Queue (LSQ)**, and a **2-bit saturating branch predictor**.

The main idea is simple: let instructions execute as early as possible, but only let them update the architectural state in program order. That keeps the machine aggressive internally while still preserving correct visible state at every cycle boundary.

## Overview

This simulator supports:

- out-of-order execution
- in-order commit
- precise exceptions
- register renaming through a RAT
- pipelined execution units with per-unit reservation stations
- sequential memory handling through an LSQ
- branch prediction with per-PC 2-bit counters

The instruction set handled by the parser and execution engine includes:

- Arithmetic: `add`, `sub`, `addi`, `mul`, `div`, `rem`
- Comparison: `slt`, `slti`
- Logic: `and`, `or`, `xor`, `andi`, `ori`, `xori`
- Memory: `lw`, `sw`
- Control flow: `beq`, `bne`, `blt`, `ble`, `j`

The input format also supports labels, memory declarations, blank lines, and `#` comments.

## How It Works

The processor advances one cycle at a time through four stages:

1. `Fetch`: fetch the instruction at `pc` and choose the next `pc` using the branch predictor
2. `Decode`: allocate an ROB entry, rename destinations through the RAT, and dispatch into the target reservation station or LSQ
3. `Execute/Broadcast`: execution units advance their pipelines and broadcast completed results on a common data bus
4. `Commit`: retire the oldest ready ROB entry and update architectural state in strict program order

The most important design choice in this codebase is that **architectural state changes only happen at commit**. That includes:

- register file writes
- memory writes
- branch correction and pipeline flushes
- making exceptions visible to the outside world

This is what keeps exceptions precise even though execution itself is out of order.

### Design Decisions

- **ROB-backed commit for correctness**
  
  Every in-flight instruction gets tracked in the ROB so retirement stays ordered even when completion does not.

- **Per-unit reservation stations**
  
  Arithmetic, logic, branch, multiplier, divider, and memory work can all wait independently for their operands and hardware availability.

- **Sequential LSQ behavior**
  
  The LSQ executes memory operations in order instead of allowing arbitrary out-of-order memory access. That simplifies correctness around aliasing and store/load interactions.

- **Store-to-load forwarding**
  
  A younger load can consume data from the most recent older uncommitted store targeting the same address, without reducing the modeled latency.

- **Per-PC 2-bit branch predictor**
  
  Each conditional branch keeps its own saturating counter. Predictor updates happen at commit time, which lines up with final branch truth.

- **Deferred exception visibility**
  
  Execution units can detect an exceptional result when they finish, but the processor only exposes the exception when that instruction reaches the head of the ROB.

## ISA and Program Semantics

### Memory declarations

Memory labels use the form:

```text
.A: 1 2 3 4
```

Multiple memory blocks are packed sequentially into the simulator's `Memory` vector.

Example:

```text
.A: 1 2 3
.B: 4 5 6
```

This maps:

- `A` to memory index `0`
- `B` to memory index `3`

### Addressing model

This project treats memory as an integer array rather than byte-addressed RAM. So advancing by one logical element changes the address by `1`, not `4`.

### Exceptions

The simulator raises exceptions for:

- arithmetic overflow on the trapping arithmetic operations
- division by zero for `div` and `rem`
- out-of-bounds memory accesses

When the oldest committing instruction has an exception:

- `pc` is set to the faulting instruction
- the public `exception` flag becomes `true`
- the rest of the pipeline is flushed
- execution halts without committing that instruction

### Register behavior

- `x0` is hardwired to `0`
- destination registers are renamed through the RAT during decode
- register results become architecturally visible only at commit

## Source Tree

### Top level

```text
.
├── Makefile
├── README.md
├── compiler.py
├── checker-col216-A2/
├── src/
└── test_cases/
```

- `Makefile`: build, run, parse, execute, test, and clean targets
- `compiler.py`: preprocessing hook used by `make run` and the checker
- `checker-col216-A2/`: testcase bundle and comparison script
- `test_cases/`: smaller local sample programs used by `make test`

### `src/`

```text
src/
├── Basics.h
├── Processor.h
├── Processor.cpp
├── main.cpp
├── BranchPredictor/
│   ├── BranchPredictor.h
│   └── BranchPredictor.cpp
├── decode_units/
│   ├── LoadStoreQueue.h
│   ├── LoadStoreQueue.cpp
│   ├── RegisterAliasTable.h
│   ├── RegisterAliasTable.cpp
│   ├── ReorderBuffer.h
│   └── ReorderBuffer.cpp
├── execute_units/
│   ├── ExecutionUnit.h
│   ├── ExecutionUnit.cpp
│   ├── ReservationStation.h
│   └── ReservationStation.cpp
└── parser/
    ├── Parser.h
    ├── Parser.cpp
    └── parser_dump.cpp
```

### What each module does

- `src/Basics.h`
  
  Defines opcodes, unit types, processor configuration, ROB entries, RS entries, and CDB result payloads.

- `src/Processor.h` and `src/Processor.cpp`
  
  The top-level processor model. This is where fetch, decode, execute/broadcast, commit, flushing, and halting are orchestrated.

- `src/main.cpp`
  
  A thin command-line driver that loads a program, runs the processor cycle-by-cycle, and prints final architectural state and memory contents.

- `src/parser/`
  
  Handles parsing of assembly-like input files, label resolution, memory-label resolution, and construction of `inst_memory`.

- `src/BranchPredictor/`
  
  Implements the branch predictor and tracks prediction statistics.

- `src/decode_units/RegisterAliasTable.*`
  
  Tracks which architectural registers currently map to in-flight ROB entries.

- `src/decode_units/ReorderBuffer.*`
  
  Holds in-flight instructions in program order, captures broadcast results, and supplies the next instruction eligible to commit.

- `src/decode_units/LoadStoreQueue.*`
  
  Handles ordered memory execution and store-to-load forwarding.

- `src/execute_units/ReservationStation.*`
  
  Generic reservation-station machinery used by the execution units.

- `src/execute_units/ExecutionUnit.*`
  
  Implements execution for the adder, multiplier, divider, branch, logic, and load/store units with configurable latency.

## Build and Usage

The repository ships with a portable `Makefile`. By default it uses `c++`, but you can override the compiler if needed:

```bash
make CXX=clang++ compile
```

### All available `make` targets

#### `make compile`

Build the simulator and produce an executable named `main`:

```bash
make compile
```

You can also compile the simulator against a custom driver file:

```bash
make compile FILE=path/to/driver.cpp
```

If `FILE` is not provided, the default driver is `src/main.cpp`.

#### `make run`

Preprocess an input assembly file in place:

```bash
make run FILE=program.s
```

Right now `compiler.py` is intentionally a no-op stub, because the parser already understands the input format directly. I kept the target because it matches the expected workflow and makes it easy to swap in a real preprocessing step later.

#### `make execute`

Run the simulator through the built executable:

```bash
make execute FILE=program.s
```

Optional flags:

```bash
make execute FILE=program.s CYCLES=50
make execute FILE=program.s VERBOSE=1
make execute FILE=program.s CYCLES=50 VERBOSE=1
```

This maps to:

```bash
./main program.s [-cycles N] [-verbose]
```

#### `make parse`

Build the parser-only debug binary and print decoded instructions:

```bash
make parse FILE=program.s
```

This is useful when you want to debug parsing or label resolution without stepping through the whole processor.

#### `make test`

Run the built simulator on all sample programs in `test_cases/`:

```bash
make test
```

This is a quick smoke test, not a strict output checker.

#### `make clean`

Remove generated binaries:

```bash
make clean
```

This deletes `main` and `parser_dump`.

## Running the Testcase Checker

For fuller validation, this repository includes a dedicated checker under `checker-col216-A2/`.

### Recommended workflow

1. Build the simulator:

```bash
make compile
```

2. Run the checker:

```bash
python3 checker-col216-A2/checker.py
```

### What the checker does

For every testcase pair, it:

- copies the source program into a temporary directory
- runs `compiler.py` on that temporary copy
- executes `./main` on the processed file
- normalizes the output
- compares it against the matching `ans*.txt`

That makes it more useful than `make test` when you want actual pass/fail validation against expected output.

## Default Processor Configuration

The default `ProcessorConfig` values in `src/Basics.h` are:

```text
Registers:          32
ROB entries:        64
Memory size:        1024

Latencies:
- Logic:            1
- Adder:            2
- Multiplier:       4
- Divider:          5
- Memory:           4
- Branch:           same as adder

Queue sizes:
- Logic RS:         4
- Adder RS:         4
- Multiplier RS:    2
- Divider RS:       2
- Branch RS:        2
- LSQ size:         32
```

These can be changed through the `ProcessorConfig` object used to construct the processor.

## Program Output

The default driver prints:

- whether execution completed naturally or stopped on an exception
- final architectural register state
- branch predictor accuracy stats
- full memory contents

That makes the default binary convenient for both debugging and automated text-based comparison.

## Notes for Users

- The simulator runs cycle-by-cycle through `Processor::step()`
- Conditional branch correctness is finalized at commit
- Jumps are redirected early through the prediction path
- The code is written to make pipeline behavior explicit and easy to inspect, rather than to mimic every detail of real hardware timing

If you want to start reading the implementation, the best entry points are `src/Processor.cpp`, `src/execute_units/ExecutionUnit.cpp`, and `src/parser/Parser.cpp`.
