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

TEST(RecordDecl, Union) {
  checkReplacements("UnionDecl.cpp", 2, "PT",
                    {21, 32, 39, 47, 56, 74, 96, 112, 119, 159, 166, 174, 239});
}

TEST(EnumDecl, Enum) {
  checkReplacements("EnumDecl.cpp", 1, "H", {5, 84, 90, 96, 181});
}

TEST(EnumDecl, EnumClass) {
  checkReplacements("EnumDecl.cpp", 1, "H", {31, 45, 140, 147, 153, 168, 193});
}

TEST(EnumConstantDecl, Enum) {
  checkReplacements("EnumDecl.cpp", 1, "D", {9, 87, 184});
  checkReplacements("EnumDecl.cpp", 1, "D", {12, 99});
  checkReplacements("EnumDecl.cpp", 1, "D", {15, 111});
}

TEST(EnumConstantDecl, EnumClass) {
  checkReplacements("EnumDecl.cpp", 1, "W", {49, 143, 196});
  checkReplacements("EnumDecl.cpp", 1, "W", {52, 156});
  checkReplacements("EnumDecl.cpp", 1, "W", {55, 171});
}

TEST(NamespaceDecl, Works) {
  checkReplacements("NamespaceDecl.cpp", 1, "a",
                    {10, 88, 106, 118, 141, 158, 279, 288, 304, 376});
  checkReplacements("NamespaceDecl.cpp", 1, "a",
                    {24, 91, 121, 161, 181, 291, 307});
}

TEST(NestedNameSpecifier, Works) {
  // rename: S
  checkReplacements("NestedNameSpecifier.cpp", 1, "a",
                    {10, 209, 237, 262, 268, 283, 297, 309});
  // rename: A
  checkReplacements("NestedNameSpecifier.cpp", 1, "a",
                    {21, 155, 174, 186, 212, 240, 271, 286, 300});
  // rename: B
  checkReplacements("NestedNameSpecifier.cpp", 1, "a",
                    {33, 133, 158, 177, 189, 215, 243, 274, 289, 303});
  // rename: C
  checkReplacements("NestedNameSpecifier.cpp", 1, "a",
                    {47, 136, 161, 192, 218, 246, 277, 292});
  // rename: W
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {80, 139, 195, 252});
  // rename: Y
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {100, 221, 320, 333});
  // rename: D
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {129});
  // rename: E
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {151});
  // rename: F
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {170, 312});
  // rename: G
  checkReplacements("NestedNameSpecifier.cpp", 1, "a", {233, 249, 317, 330});
}

TEST(Templates, Works) {
  // Make this test better
  checkReplacements("Template.cpp", 1, "U", {19, 46});
  checkReplacements("Template.cpp", 1, "U", {88, 115});
}
