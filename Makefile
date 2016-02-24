LLVM:=$(HOME)/repos/llvm/llvm
BUILD:=$(HOME)/repos/llvm/build

LIBPATHS:=-L$(BUILD)/lib
LIBS:=-lclangBasic -lclangLex -lclangIndex -lclangTooling -lclangAST -lclangASTMatchers
INCS:=-I$(LLVM)/tools/clang/include -I$(BUILD)/tools/clang/include
#INCS:=$(INCS) -I$(LLVM)/include -I$(BUILD)/include
# CXX:=$(HOME)/repos/llvm/build/bin/clang++
CXX:=clang++

CXXFLAGS:=-std=c++11

rn: Rename.cpp.o
	$(CXX) `$(BUILD)/bin/llvm-config --ldflags` $(CXXFLAGS) $(DEFINES) -o rn Rename.cpp.o `$(BUILD)/bin/llvm-config --system-libs --libs support` $(LIBPATHS) $(LIBS) $(INCS)

Rename.cpp.o: Rename.cpp
	clang++ `$(BUILD)/bin/llvm-config --cxxflags` $(INCS) -c -o Rename.cpp.o Rename.cpp
	#!$(CXX) $(CXXFLAGS) -c -o Rename.cpp.o Rename.cpp $(INCS)
