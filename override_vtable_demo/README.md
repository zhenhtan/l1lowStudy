# override / virtual / vtable demo

## Build

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o demo
```

## Run

```bash
./demo
```

## What this demo shows

1. `override` catches signature mismatch at compile time.
2. `virtual` enables runtime polymorphism through base references/pointers.
3. `vptr` address printing gives an intuitive view of vtable-based dispatch (implementation-specific, for learning only).
