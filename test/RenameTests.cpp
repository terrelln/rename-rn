#include "RenameTestHarness.h"

#include <clang/Tooling/Refactoring.h>

#include <gtest/gtest.h>

#include <string>

using namespace clang::tooling;

TEST(VarDecl, Works) {
  std::string File = addPrefix("VarDecl.cpp");
  std::string NewSpelling = "hey";
  Replacements Replaces = {Replacement(File, 36, 1, NewSpelling),
                           Replacement(File, 41, 1, NewSpelling),
                           Replacement(File, 58, 1, NewSpelling),
                           Replacement(File, 62, 1, NewSpelling),
                           Replacement(File, 74, 1, NewSpelling)};
  RunResults ActualResults = runRenaming(File, 5, 7, NewSpelling);
  RunResults ExpectedResults(std::move(Replaces));
  EXPECT_EQ(ExpectedResults, ActualResults);
}
