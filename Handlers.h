#include "Utility.h"

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Tooling/Refactoring.h>

#include <llvm/ADT/Optional.h>

namespace rn {

// Data about the Symbol that the Matcher callbacks need
struct SymbolData {
  SymbolData(std::string File, unsigned Line, unsigned Column,
             std::string NewSpelling)
      : File(std::move(File)), Line(Line), Column(Column),
        NewSpelling(std::move(NewSpelling)) {}

  std::string File;
  unsigned Line;
  unsigned Column;
  std::string NewSpelling;

  ::llvm::Optional<::clang::SourceLocation> Loc;
  std::string USR;
  std::string Spelling;
};

template <typename AnnotatedNode>
class RenameHandler : public ::clang::ast_matchers::MatchFinder::MatchCallback {
public:
  RenameHandler(::clang::tooling::Replacements *Replace, const SymbolData *Data)
      : Replace(Replace), Data(Data) {}

  void
  run(const ::clang::ast_matchers::MatchFinder::MatchResult &Result) override {
    // Rename the Node if there is a match
    if (const auto Node =
            Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
                AnnotatedNode::ID())) {
      Replace->insert(::clang::tooling::Replacement(
          *(Result.SourceManager), AnnotatedNode::getLocation(Node),
          Data->Spelling.size(), Data->NewSpelling));
    }
  }

private:
  ::clang::tooling::Replacements *Replace;
  const SymbolData *Data;
};

template <typename AnnotatedNode>
class SourceLocationHandler
    : public ::clang::ast_matchers::MatchFinder::MatchCallback {
public:
  SourceLocationHandler(SymbolData *Data)
      : Data(Data), AlreadyMatchedThisNode(false) {}

  virtual void
  run(const ::clang::ast_matchers::MatchFinder::MatchResult &Result) {
    const auto SourceMgr = Result.SourceManager;
    if (SourceMgr == nullptr)
      return;
    // Get the matched node
    const auto Node = Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
        AnnotatedNode::ID());
    if (Node == nullptr)
      return;
    // and its NamedDecl
    const auto Decl = AnnotatedNode::getNamedDecl(Node);
    if (Decl == nullptr)
      return;
    // See if it is at the location we are looking for
    check(*SourceMgr, Decl, AnnotatedNode::getLocation(Node));
  }

private:
  void check(const ::clang::SourceManager &SourceMgr,
             const ::clang::NamedDecl *Decl,
             const ::clang::SourceLocation &Start) {
    const auto Length = Decl->getNameAsString().length();
    if (Length == 0)
      return;
    const auto End = Start.getLocWithOffset(Length - 1);
    // If the location is in an expanded macro, we do not want to rename it.
    // If the Data->Loc isn't between Start and End or if either SourceLocation
    // is invalid or in an expanded macro its no good
    if (!Start.isValid() || !End.isValid() || Start.isMacroID() ||
        End.isMacroID() || !isLocWithin(SourceMgr, Start, End))
      return;
    // This is a workaround for RecordDecl's with definitions, since we want the
    // outer matcher.
    if (AlreadyMatchedThisNode)
          return;
    Data->USR = getUSRForDecl(Decl);
    Data->Spelling = Decl->getNameAsString();
    AlreadyMatchedThisNode = true;
  }

  // Returns true if Data->Loc is within [Start, End].
  bool isLocWithin(const ::clang::SourceManager &SourceMgr,
                   const ::clang::SourceLocation &Start,
                   const ::clang::SourceLocation &End) {
    // We only have a SourceManager in the callback, so if the Loc hasn't been
    // translated from <file>:<line>:<col> yet, translate it and save the data.
    if (!Data->Loc.hasValue()) {
      const auto *FileEntry = SourceMgr.getFileManager().getFile(Data->File);
      if (FileEntry == nullptr) {
        return false;
      }
      Data->Loc =
          SourceMgr.translateFileLineCol(FileEntry, Data->Line, Data->Column);
    }
    const auto Loc = *(Data->Loc);
    if (!Loc.isValid()) {
      return false;
    }
    return Loc == Start || Loc == End ||
           (SourceMgr.isBeforeInTranslationUnit(Start, Loc) &&
            SourceMgr.isBeforeInTranslationUnit(Loc, End));
  }

  SymbolData *Data;
  bool AlreadyMatchedThisNode;
};
}
