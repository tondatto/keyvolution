# keyvolution
Optimizing keyboard layouts to minimize finger swipe distances using genetic algorithms in C and Python.

## Usage

You must provide a file with a list of words as a parameter.

### Python 

```shell
$ python3 ./best-layout-ga.py top400-pt.txt
```

### C

Compile the code with GCC `gcc -o best-layout-ga best-layout-ga.c -lm`.

```shell
$ ./best-layout-ga top400-pt.txt
```
