#include "RenameTestHarness.h"

#include "../Handlers.h"
#include "../Matchers.h"
#include "../Nodes.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Refactoring.h>

#include <fstream>
#include <string>
#include <vector>

using namespace clang;

using clang::tooling::FixedCompilationDatabase;
using clang::tooling::RefactoringTool;
using clang::tooling::Replacements;
using clang::tooling::newFrontendActionFactory;

using clang::ast_matchers::MatchFinder;

std::string addPrefix(std::string File) {
  const std::string Directory = "test/files/";
  return Directory + File;
}

std::ostream &operator<<(std::ostream &out, const RunResults &Results) {
  out << "\nSourceLocationProcessingFailed: "
      << Results.SourceLocationProcessingFailed
      << "\nUnableToDetermineUSR: " << Results.UnableToDetermineUSR
      << "\nRenameProcessingFailed: " << Results.RenameProcessingFailed
      << "\nReplacements:";
  for (const auto &Replace : Results.Replaces) {
    out << "\n\t" << Replace.toString();
  }
  return out;
}
RunResults runRenaming(std::string File, unsigned Line, unsigned Column,
                       std::string NewSpelling) {
  RunResults Results;
  using namespace rn;

  SymbolData Data(File, Line, Column, NewSpelling);

  std::string ErrorMsg;
  std::vector<std::string> Args;
  Args.push_back("-std=c++11");
  auto CompilationDB = FixedCompilationDatabase{".", Args};

  std::vector<std::string> Files;
  Files.push_back(File);
  RefactoringTool Tool(CompilationDB, Files);

  // Find the source location
  {
    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_SOURCE_LOCATION_MATCHER)

    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      Results.SourceLocationProcessingFailed = true;
      return Results;
    }
  }
  if (Data.USR.empty()) {
    Results.UnableToDetermineUSR = true;
    return Results;
  }

  // Find all references and rename them
  {
    auto Replace = &Tool.getReplacements();
    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_RENAME_MATCHER)
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      Results.RenameProcessingFailed = true;
      return Results;
    }
  }
  Results.Replaces = Tool.getReplacements();
  return Results;
}

namespace {
void getLineColumn(std::string File, unsigned Offset, unsigned *Line,
                   unsigned *Column) {
  if (Line == nullptr || Column == nullptr)
    return;
  *Line = 1;
  std::string Buffer;
  std::ifstream In{File};
  while (std::getline(In, Buffer)) {
    if (Buffer.length() + 1 > Offset) {
      *Column = Offset + 1;
      return;
    }
    ++(*Line);
    Offset -= Buffer.length() + 1;
    Buffer.clear();
  }
  *Column = Offset + 1;
}
}

RunResults runRenaming(std::string File, unsigned Offset,
                       std::string NewSpelling) {
  unsigned Line, Column;
  getLineColumn(File, Offset, &Line, &Column);
  return runRenaming(File, Line, Column, NewSpelling);
}
