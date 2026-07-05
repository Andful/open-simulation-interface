# Open Simulator Interface

An C-RTL co-simulation interface. 

## Features
* Written in C:  
    C is the lingua franca of computers.
* Single header library:  
    All the code is contained within `include/osi.h`.
* User driven simulation:  
    In contrast, VPI, the user can only attach callbacks. This makes developer
    experience, less than ideal.
* Scaffolding to support of multiple simulators:  
    Currently only XSI is supported, but scafolding is present, through `t_simulation` to support different simulators.

## Planned (Not Yet Implemented) Features
* Support multiple simulators:  
    Currently, only XSI is supported. Planned support for Verilator and Icarus Verilog.
* Support for VHDL

## Not Planned Features
* Hierarchical access to signals and ports:  
    Only access top module access to signal and ports is supported.
    Many simulators don't support hierarchical access to signals and ports by default, such as XSI and Verilator.