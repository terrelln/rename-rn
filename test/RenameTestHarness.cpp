#include "RenameTestHarness.h"

#include "../Handlers.h"
#include "../Matchers.h"
#include "../Nodes.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Refactoring.h>

#include <set>
#include <utility>
#include <vector>

using clang::tooling::CompilationDatabase;
using clang::tooling::RefactoringTool;
using clang::tooling::Replacements;
using clang::tooling::newFrontendActionFactory;

using clang::ast_matchers::MatchFinder;

RunResults RunRenaming(std::string File, unsigned Line, unsigned Column,
                       std::string NewSpelling) {
  RunResults Results;
  using namespace rn;

  SymbolData Data(File, Line, Column, NewSpelling);

  std::string ErrorMsg;
  auto CompilationDB =
      CompilationDatabase::autoDetectFromSource(File, ErrorMsg);

  std::vector<std::string> Files = {File};
  RefactoringTool Tool(*CompilationDB, Files);

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
