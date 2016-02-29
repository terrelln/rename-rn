LLVM:=$(HOME)/repos/llvm/llvm
BUILD:=$(HOME)/repos/llvm/build

LIBPATHS:=-L$(BUILD)/lib
LIBS:=-lclangAST -lclangASTMatchers -lclangBasic -lclangFrontend \
  -lclangLex -lclangRewrite -lclangSema -lclangTooling \
  -lclangToolingCore -lclangIndex -lclangFrontendTool \
  -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers \
  -lclangStaticAnalyzerCore -lclangAnalysis \
  -lclangRewriteFrontend -lclangEdit
INCS:=-I$(LLVM)/tools/clang/include -I$(BUILD)/tools/clang/include
CXX:=clang++

CXXFLAGS:=-std=c++11 -g

all: rn test

rn: Rename.cpp.o
	$(CXX) `$(BUILD)/bin/llvm-config --ldflags` $(CXXFLAGS) $(DEFINES) -o rn Rename.cpp.o `$(BUILD)/bin/llvm-config --system-libs --libs` $(LIBPATHS) $(LIBS) $(INCS)

Rename.cpp.o: Rename.cpp Handlers.h Matchers.h Nodes.h Utility.h
	$(CXX) $(CXXFLAGS) `$(BUILD)/bin/llvm-config --cxxflags` $(INCS) -c -o Rename.cpp.o Rename.cpp

test/RenameTestHarness.cpp.o: test/RenameTestHarness.cpp test/RenameTestHarness.h Handlers.h Matchers.h Nodes.h Utility.h
	$(CXX) $(CXXFLAGS) `$(BUILD)/bin/llvm-config --cxxflags` $(INCS) -c -o test/RenameTestHarness.cpp.o test/RenameTestHarness.cpp

test/RenameTests.cpp.o: test/RenameTests.cpp Handlers.h Matchers.h Nodes.h Utility.h
	$(CXX) $(CXXFLAGS) `$(BUILD)/bin/llvm-config --cxxflags` $(INCS) -isystem test/googletest/googletest/include -pthread -c -o test/RenameTests.cpp.o test/RenameTests.cpp 
	
test: test/RenameTestHarness.cpp.o test/RenameTests.cpp.o
	$(CXX) `$(BUILD)/bin/llvm-config --ldflags` $(CXXFLAGS) $(DEFINES) -o test/test test/RenameTestHarness.cpp.o test/RenameTests.cpp.o test/gtest_main.a -pthread `$(BUILD)/bin/llvm-config --system-libs --libs` $(LIBPATHS) $(LIBS) $(INCS)
	
.PHONY=clean
clean:
	rm -f test/*.o test/test *.o rn
