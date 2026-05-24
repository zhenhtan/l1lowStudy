# CRTP demo

Curiously Recurring Template Pattern: compile-time polymorphism vs. virtual functions.

## Build

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o demo
```

## Run

```bash
./demo
```

## What this demo shows

1. **CRTP (Static Polymorphism)**
   - Base class is a template parameterized by the derived class.
   - `AnimalCRTP<DogCRTP>`: Dog knows it's a Dog at compile time.
   - Dispatch resolved via `static_cast` at compile time; no runtime overhead.
   - Compiler can inline all calls; very efficient.

2. **Virtual (Dynamic Polymorphism)**
   - Base class is not a template; works with pointers/references.
   - Runtime dispatch via vtable indirection.
   - Can store heterogeneous objects in a single container (unique_ptr<Base>).

3. **Trade-offs**
   - **CRTP**: Zero overhead, inline-friendly, but type information is lost at runtime; harder to store mixed types in containers.
   - **Virtual**: Small overhead per call (vtable indirection), but flexible for heterogeneous collections and explicit runtime dispatch.

## Notes

- CRTP is common in performance-critical code and template libraries (e.g., STL, Boost).
- Virtual functions are better for when you need true runtime type polymorphism.
- Sometimes both patterns are combined: template + virtual for maximum flexibility.
