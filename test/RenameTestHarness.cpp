#include "RenameTestHarness.h"

#include "../Handlers.h"
#include "../Matchers.h"
#include "../Nodes.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Refactoring.h>

#include <string>
#include <vector>

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
