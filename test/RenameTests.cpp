#include "RenameTestHarness.h"

#include <clang/Tooling/Refactoring.h>

#include <gtest/gtest.h>

#include <string>

using namespace std;
using namespace clang::tooling;

TEST(VarDecl, Works) {
  string File = addPrefix("VarDecl.cpp");
  string NewSpelling = "hey";
  Replacements XReplaces = {Replacement{File, 36, 1, NewSpelling},
                            Replacement{File, 41, 1, NewSpelling},
                            Replacement{File, 58, 1, NewSpelling},
                            Replacement{File, 62, 1, NewSpelling},
                            Replacement{File, 74, 1, NewSpelling}};
  RunResults ExpectedResults(std::move(XReplaces));

  RunResults ActualResults = runRenaming(File, 5, 7, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 6, 3, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 7, 11, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 7, 15, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 8, 10, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  // Check `yss`
  Replacements YssReplaces = {Replacement{File, 83, 3, NewSpelling}};
  ExpectedResults.Replaces = std::move(YssReplaces);

  for (unsigned col = 5; col < 8; ++col) {
    ActualResults = runRenaming(File, 10, col, NewSpelling);
    EXPECT_EQ(ExpectedResults, ActualResults);
  }
}

TEST(TagDecl, RecordDecl) {
  string File = addPrefix("RecordDecl.cpp");
  string NewSpelling = "PT";
  Replacements Replaces = {Replacement{File, 21, 2, NewSpelling},
                           Replacement{File, 32, 2, NewSpelling},
                           Replacement{File, 39, 2, NewSpelling},
                           Replacement{File, 47, 2, NewSpelling},
                           Replacement{File, 56, 2, NewSpelling},
                           Replacement{File, 74, 2, NewSpelling},
                           Replacement{File, 96, 2, NewSpelling},
                           Replacement{File, 112, 2, NewSpelling},
                           Replacement{File, 119, 2, NewSpelling},
                           Replacement{File, 159, 2, NewSpelling},
                           Replacement{File, 166, 2, NewSpelling},
                           Replacement{File, 174, 2, NewSpelling},
                           Replacement{File, 239, 2, NewSpelling}};
  RunResults ExpectedResults{std::move(Replaces)};

  RunResults ActualResults = runRenaming(File, 2, 8, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 3, 8, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 4, 3, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 5, 3, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 5, 4, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 5, 12, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 6, 12, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 7, 4, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 9, 10, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 16, 1, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 16, 8, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 16, 16, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);

  ActualResults = runRenaming(File, 24, 10, NewSpelling);
  EXPECT_EQ(ExpectedResults, ActualResults);
}
