# Zombit

A novel compact data structure for bitvectors with *k*
runs that achieves access, rank, and successor in *O(1)* time by consuming space
*O(√kn)* bits. That work was presented in [1].

## Installation
```
git clone https://github.com/adriangbrandon/zombit.git
cd zombit
./compile.sh
```

## Test Zombit  
After the compilation stage, folder `build` should have an executable `main to test the structure:
```
cd build/ 
./main <size> <test>
```
Where the executable `main` has two options:
1. **size**: the length of a random generated bitmap.
2. **test**: this parameter can be `0` or `1`. In case of `0` it only builds the structure and performs successor queries on it. With the second option, the executable checks that every returned value is correct.

## Authors
Adrián Gómez Brandón ([adrian.gbrandon@udc.es](mailto:adrian.gbrandon@udc.es))

## References

[1] Gómez-Brandón, A. (2020, March). Bitvectors with runs and the successor/predecessor problem. In 2020 Data Compression Conference (DCC) (pp. 133-142). IEEE.







