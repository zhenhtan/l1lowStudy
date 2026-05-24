# Keil Compile Demo (Keil-style structure, runnable on Linux)

## What is Keil compilation?
Keil MDK compilation for embedded projects typically includes these stages:
1. Preprocess (`.c` -> expanded source)
2. Compile (`.c` -> `.o`)
3. Assemble (`.s` -> `.o`, if any assembly files)
4. Link (`.o` + linker script/scatter file -> `.axf`/`.elf`)
5. Convert (`fromelf`, optional: `.bin`/`.hex`)

In short: **each source file is compiled independently, then linked into one firmware image**.

## Why this demo
This demo mimics Keil project layering:
- `main.c`: application logic
- `system_init.c`: system init and delay API
- `bsp_led.c`: board support package

It runs directly on Linux so you can verify behavior immediately.

## Build and run (Linux)
```bash
cd leaningProgram/keil_compile_demo
make clean && make run
```

## Example Keil command line (Windows, reference)
If `UV4.exe` is in PATH:
```bat
UV4 -b keil_compile_demo.uvprojx -t "Target 1" -j0
```
- `-b`: build
- `-t`: target name in Keil project
- `-j0`: use all cores

## Expected output
You should see initialization logs and LED ON/OFF toggling logs.
