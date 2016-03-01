#include "RenameTestHarness.h"

#include <clang/Tooling/Refactoring.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace std;
using namespace clang::tooling;

void checkReplacements(string File, unsigned SpellingLength, string NewSpelling,
                       const std::vector<unsigned> &Locs) {
  File = addPrefix(File);
  Replacements Replaces;
  for (const auto Loc : Locs) {
    Replaces.emplace(File, Loc, SpellingLength, NewSpelling);
  }
  RunResults ExpectedResults{std::move(Replaces)};
  RunResults ActualResults;

  for (const auto Loc : Locs) {
    EXPECT_EQ(ExpectedResults, runRenaming(File, Loc, NewSpelling));
  }
}

TEST(VarDecl, Works) {
  checkReplacements("VarDecl.cpp", 1, "hey", {36, 41, 58, 62, 74});
}

TEST(RecordDecl, Struct) {
  checkReplacements("RecordDecl.cpp", 2, "PT",
                    {21, 32, 39, 47, 56, 74, 96, 112, 119, 159, 166, 174, 239});
}

TEST(EnumDecl, Enum) {
  checkReplacements("EnumDecl.cpp", 1, "H", {5, 84, 90, 96, 181});
}

TEST(EnumDecl, EnumClass) {
  checkReplacements("EnumDecl.cpp", 1, "H", {31, 45, 140, 147, 153, 168, 193});
}
