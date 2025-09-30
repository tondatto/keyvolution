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

## Best results

Running the program on a machine with Linux Pop!_OS 22.04 LTS operating system and Intel® Core™ i7-12700 processor this is the best results for the [400 most common portuguese words][1]:

Python (time: 242s)
```
K F G N O T S P X Y
 J H C A R E M B W
  Q U L I D V Z
Average distance per word: 1.8930
```

C (time: 61s)
```
K Y Z M A R S C V X
 W J B T E O I L P
  Q F U D N G H
Average distance per word: 1.9060
```

[1]: https://portuguesewithcarla.com/1000-most-common-portuguese-words/
