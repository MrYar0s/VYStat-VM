# Shrimp VM
Toy vm for student course

## ISA

* instr.yaml contains ISA description
* instr.py is a helper library for ISA processing

## Modules

1) assembler
2) runtime - runtime library
3) shrimp - vm application
3) shrimpfile - vm executables format library
4) tests

## How to build

```bash
mkdir build
cd build
cmake ..
make -j16
```

## How to run (after build)

* Assembler executable can be found here: ```build/assembler/assembler``` . To run:

```bash
./assembler --in <input (.shr) file name> --out <out (.imp) file name>
```

* VM executable can be found here: ```build/shrimp/shrimp``` . To run:

```bash
./shrimp --in <input (.imp) file name>
```

Test .shr sources can be found in tests/e2e

* To run tests:

```bash
make tests
```
