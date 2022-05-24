@echo off

g++ -Ofast -s .\Main.cpp -I src/ZerEngine -std=c++2a -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wformat=2 -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wdouble-promotion -Wnull-dereference -Wuseless-cast -Wsign-conversion -Wpedantic -Wconversion