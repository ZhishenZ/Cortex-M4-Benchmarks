# Cortex-M4-Thesis-Benchmarks

This repo contains the benchmarks from thesis “Boosting Cortex-M Microcontroller Time and Energy Efficiency for Memory-intensive Applications”

Since in some tests, instructions and data are both allocated on the CCM-RAM, 
the CCM-RAM is divided into two sections `.ccmram` and `.ccmramdata` in this case by changing the linker script `*.ld`. The directories with name ending "ccm" are the benchmarks, in which CCM-RAM is divided into two sections. 

The allocation of instructions and data are modified by the macro. 

The code for benchmarks are in the directory `Cortex-M4-Thesis-Benchmarks/benchmark_name/Core/Src`