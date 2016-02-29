#pragma once

#include <clang/Tooling/Refactoring.h>

#include <iostream>
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

  RunResults(const RunResults &) = default;
  RunResults(RunResults &&) = default;

  RunResults &operator=(RunResults &&RHS) {
    SourceLocationProcessingFailed = RHS.SourceLocationProcessingFailed;
    UnableToDetermineUSR = RHS.UnableToDetermineUSR;
    RenameProcessingFailed = RHS.RenameProcessingFailed;
    Replaces = std::move(RHS).Replaces;
    return *this;
  }

  bool operator==(const RunResults &RHS) const {
    return SourceLocationProcessingFailed ==
               RHS.SourceLocationProcessingFailed &&
           UnableToDetermineUSR == RHS.UnableToDetermineUSR &&
           RenameProcessingFailed == RHS.RenameProcessingFailed &&
           Replaces == RHS.Replaces;
  }

  bool SourceLocationProcessingFailed;
  bool UnableToDetermineUSR;
  bool RenameProcessingFailed;
  ::clang::tooling::Replacements Replaces;
};

std::ostream &operator<<(std::ostream &out, const RunResults &Results);

std::string addPrefix(std::string File);

RunResults runRenaming(std::string File, unsigned Line, unsigned Column,
                       std::string NewSpelling);
