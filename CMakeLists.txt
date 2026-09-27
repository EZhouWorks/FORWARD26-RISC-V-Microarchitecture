cmake_minimum_required(VERSION 3.20)

project(RISCV_CPU)

set(CMAKE_CXX_STANDARD 20)

add_executable(RISCV_CPU
        test.cpp
        CPU/CPU.cpp
        src/ForwardingUnit.h
        src/StallUnit.h
        src/BranchUnit.h
        src/JumpUnit.h
        src/Probe.h
)