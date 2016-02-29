#pragma once

#include <clang/Tooling/Refactoring.h>

#include <string>
#include <utility>

struct RunResults {
  RunResults()
      : SourceLocationProcessingFailed(false), UnableToDetermineUSR(false),
        RenameProcessingFailed(false),
        Replaces(::clang::tooling::Replacements{}) {}

  explicit RunResults(::clang::tooling::Replacements Replaces)
      : SourceLocationProcessingFailed(false), UnableToDetermineUSR(false),
        RenameProcessingFailed(false), Replaces(std::move(Replaces)) {}
  bool SourceLocationProcessingFailed;
  bool UnableToDetermineUSR;
  bool RenameProcessingFailed;
  ::clang::tooling::Replacements Replaces;
};

RunResults RunRenaming(std::string File, unsigned Line, unsigned Column,
                       std::string NewSpelling);
