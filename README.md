# Hash Hash Hash
We built a hash table with mutexes to handle race conditions and collisions.
## Building
```shell
make clean && make
```

## Running
```shell
./hash-table-tester -t 8 -s 50000

Generation: 72,712 usec
Hash table base: 1,037,472 usec
  - 0 missing

```

## First Implementation
In the `hash_table_v1_add_entry` function, I added a single mutex at lines 76-119,
I blocked adding values as a critical section, guaranteeing correctness because 
all operations are protected from two threads adding to the hash table at the 
exact same time.

### Performance
```shell
./hash-table-tester -t 8 -s 50000

Generation: 72,712 usec
Hash table base: 1,037,472 usec
  - 0 missing
Hash table v1: 1,435,129 usec
  - 0 missing
Hash table v2: 372,111 usec
  - 0 missing
```
Version 1 is a slower than the base version because there is some 
overhead caused by creating, locking and unlocking threads, resulting in a bit 
slower execution. Locking adding as a critical section results in no parallelism.

## Second Implementation
In the `hash_table_v2_add_entry` function, I add one multiplex per hash table entry
in order to ensure performance and correctness of the hash table. The mutexes
block adding to each entry so that other bukcets can still take entries, allowing 
parallelism. Two threads can't simulanesouly insert at the head, nor can they
insert two keys with the same value, so it ensures correctness. I put the locks
around 

### Performance
```shell
TODO how to run and results
./hash-table-tester -t 8 -s 50000
```

TODO more results, speedup measurement, and analysis on v2
The time the speedup is approximately (how many times faster)x, similar to 
the number of physical cores(how many cores).The speedup owes to (which sessions
contribute to parallelization)

## Cleaning up
```shell
make clean
```