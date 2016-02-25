#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace clang::tooling;

using llvm::errs;
using llvm::outs;
using llvm::Optional;
using llvm::StringRef;

namespace rn {

bool isSameDecl(const Decl &First, const Decl &Second) {
  return First.getCanonicalDecl() == Second.getCanonicalDecl();
}

AST_MATCHER_P(Decl, singularNamedDeclNode, const NamedDecl *, Declaration) {
  if (Declaration == nullptr)
    return false;
  return isSameDecl(Node, *Declaration);
}

StatementMatcher declRefMatcher(const NamedDecl &Decl) {
  return declRefExpr(hasDeclaration(singularNamedDeclNode(&Decl)));
}

class DeclRefHandler : public MatchFinder::MatchCallback {
public:
  DeclRefHandler(Replacements *Replace, StringRef NewSpelling)
      : Replace(Replace), NewSpelling(NewSpelling) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const DeclRefExpr *Ref = Result.Nodes.getNodeAs<DeclRefExpr>("expr")) {
      Replacement Rep(*(Result.SourceManager), Ref, NewSpelling);
      Replace->insert(Rep);
    }
  }

private:
  Replacements *Replace;
  StringRef NewSpelling;
};

class SourceLocationHandler : public MatchFinder::MatchCallback {
public:
  SourceLocationHandler(Replacements *Replace, StringRef NewSpelling,
                        StringRef File, unsigned Offset)
      : Replace(Replace), NewSpelling(NewSpelling), File(File), Offset(Offset),
        Decl(nullptr) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    // FIXME: This is an ugly way to get the source location.
    const auto SourceMgr = Result.SourceManager;
    if (SourceMgr == nullptr)
      return;
    if (!Loc.hasValue()) {
      // Fixme, use File
      Loc = SourceMgr->getLocForStartOfFile(SourceMgr->getMainFileID())
                .getLocWithOffset(Offset);
    }
    if (!Loc->isValid()) {
      return;
    }
    if (const DeclRefExpr *Ref = Result.Nodes.getNodeAs<DeclRefExpr>("expr")) {
      if (const NamedDecl *D = Ref->getFoundDecl())
        setResult(*SourceMgr, D, Ref->getLocation(),
                  lenToLoc(Ref->getLocation(), D->getNameAsString().length()));
    } else if (const NamedDecl *D = Result.Nodes.getNodeAs<NamedDecl>("decl")) {
      setResult(*SourceMgr, D, D->getLocation(),
                lenToLoc(D->getLocation(), D->getNameAsString().length()));
    }
  }

  const NamedDecl *getNamedDecl() const { return Decl; }

private:
  SourceLocation lenToLoc(const SourceLocation &Start, unsigned Length) {
    if (Length == 0)
      return SourceLocation{};
    return Start.getLocWithOffset(Length - 1);
  }

  void setResult(const SourceManager &SourceMgr, const NamedDecl *D,
                 const SourceLocation &Start, const SourceLocation &End) {
    // If the location is in an expanded macro, we do not want to rename it.
    if (!Start.isValid() || !End.isValid() || Start.isMacroID() ||
        End.isMacroID() || !isLocWithin(SourceMgr, Start, End))
      return;
    Replacement Rep(SourceMgr, D, NewSpelling);
    Replace->insert(Rep);
    Decl = D;
  }

  bool isLocWithin(const SourceManager &SourceMgr, const SourceLocation &Start,
                   const SourceLocation &End) {
    return *Loc == Start || *Loc == End ||
           (SourceMgr.isBeforeInTranslationUnit(Start, *Loc) &&
            SourceMgr.isBeforeInTranslationUnit(*Loc, End));
  }

  Replacements *Replace;
  StringRef NewSpelling;
  StringRef File;
  unsigned Offset;
  Optional<SourceLocation> Loc;
  const NamedDecl *Decl;
};
}

// Options
llvm::cl::OptionCategory RenameCategory("rn options");

static llvm::cl::opt<std::string>
    NewSpelling("new-name",
                llvm::cl::desc("The new name to change the symbol to."),
                llvm::cl::cat(RenameCategory));

static llvm::cl::opt<unsigned>
    Offset("offset",
           llvm::cl::desc(
               "Locates the symbol by offset as opposed to <line>:<column>."),
           llvm::cl::cat(RenameCategory));

const std::string CLANG_RENAME_VERSION = "0.0.1";

static void PrintVersion() {
  outs() << "clang-rename version " << CLANG_RENAME_VERSION << "\n";
}

const char RenameUsage[] = "A tool to rename symbols in C/C++ code.\n\
                            rn renames every occurrence of a symbol found at <offset> in\n\
                            <source>. The results are written to stdout.\n";

int main(int argc, const char **argv) {
  using namespace rn;

  llvm::cl::SetVersionPrinter(PrintVersion);
  tooling::CommonOptionsParser OP(argc, argv, RenameCategory, RenameUsage);

  if (NewSpelling.empty()) {
    errs() << "rn: no new name provided.\n\n";
    llvm::cl::PrintHelpMessage();
    exit(1);
  }

  auto Files = OP.getSourcePathList();
  if (Files.empty()) {
    errs() << "rn: no files provided.\n\n";
    llvm::cl::PrintHelpMessage();
    exit(1);
  }

  tooling::RefactoringTool Tool(OP.getCompilations(), Files);

  const NamedDecl *ReferencedDecl = nullptr;
  {
    SourceLocationHandler HandlerForSourceLoc(
        &Tool.getReplacements(), NewSpelling, Files.front(), Offset);
    MatchFinder Finder;
    Finder.addMatcher(namedDecl().bind("decl"), &HandlerForSourceLoc);
    Finder.addMatcher(declRefExpr().bind("expr"), &HandlerForSourceLoc);
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to find symbol in file: \"" << Files.front()
             << "\" at offset: " << Offset << ".\n";
      exit(1);
    }
    ReferencedDecl = HandlerForSourceLoc.getNamedDecl();
  }
  if (ReferencedDecl == nullptr) {
    errs() << "Failed to find symbol in file: \"" << Files.front()
           << "\" at offset: " << Offset << ".\n";
    exit(1);
  }

  // Find all references and rename them
  {
    DeclRefHandler HandleDeclRef(&Tool.getReplacements(), NewSpelling);
    MatchFinder Finder;
    Finder.addMatcher(declRefMatcher(*ReferencedDecl), &HandleDeclRef);
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to rename symbol in file: \"" << Files.front()
             << "\" at offset: " << Offset << ".\n";
    }
  }
  llvm::outs() << "Replacements collected by the tool:\n";
  for (auto &r : Tool.getReplacements()) {
    llvm::outs() << r.toString() << "\n";
  }

  return 0;
}
