#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Index/USRGeneration.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace clang::tooling;

using llvm::errs;
using llvm::outs;
using llvm::StringRef;

namespace rn {

std::string getUSRForDecl(const NamedDecl &Decl) {
  llvm::SmallVector<char, 128> Buf;

  if (index::generateUSRForDecl(&Decl, Buf))
    return std::string{};

  return std::string(Buf.data(), Buf.size());
}

AST_MATCHER_P(NamedDecl, sameUSR, StringRef, USR) {
  return getUSRForDecl(Node) == USR;
}

struct NamedDeclNode {
  using NodeType = clang::NamedDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "NamedDecl"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) { return Node; }

  static const MatcherType matchNode() { return namedDecl().bind(ID()); }

  static const DeclarationMatcher
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return namedDecl(InnerMatcher).bind(ID());
  }
};

struct DeclRefExprNode {
  using NodeType = clang::DeclRefExpr;
  using MatcherType = StatementMatcher;
  static constexpr const char *ID() { return "DeclRefExpr"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getFoundDecl();
  }

  static const MatcherType matchNode() { return declRefExpr().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return declRefExpr(hasDeclaration(namedDecl(InnerMatcher))).bind(ID());
  }
};

/*
StatementMatcher declRefMatcher(StringRef USR) {
  return declRefExpr(hasDeclaration(namedDecl(sameUSR(USR))))
      .bind(DeclRefExprNode::ID());
}

DeclarationMatcher namedDeclMatcher(StringRef USR) {
  return namedDecl(sameUSR(USR)).bind(NamedDeclNode::ID());
}

namespace internal {

template <typename...> struct RunImpl;

template <> struct RunImpl<> {
  static void run(Replacements *Replace, StringRef Spelling,
                  StringRef NewSpelling,
                  const MatchFinder::MatchResult &Result) {}
};

template <typename AnnotatedNode, typename... Rest>
struct RunImpl<AnnotatedNode, Rest...> {
  static void run(Replacements *Replace, StringRef Spelling,
                  StringRef NewSpelling,
                  const MatchFinder::MatchResult &Result) {
    errs() << "Trying id: " << AnnotatedNode::ID() << "\n";
    if (const auto Node =
            Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
                AnnotatedNode::ID())) {
      errs() << "Found match with id: " << AnnotatedNode::ID() << "\n";
      Replace->insert(Replacement(*(Result.SourceManager), Node->getLocation(),
                                  Spelling.size(), NewSpelling));
    } else {
      RunImpl<Rest...>::run(Replace, Spelling, NewSpelling, Result);
    }
  }
};

template <typename... AnnotatedNodes>
class RenameHandlerImpl : public MatchFinder::MatchCallback {
public:
  RenameHandlerImpl(Replacements *Replace, StringRef Spelling,
                    StringRef NewSpelling)
      : Replace(Replace), Spelling(Spelling), NewSpelling(NewSpelling) {}

  void run(const MatchFinder::MatchResult &Result) override {
    errs() << "Calling impl\n";
    RunImpl<AnnotatedNodes...>::run(Replace, Spelling, NewSpelling, Result);
  }

private:
  Replacements *Replace;
  StringRef Spelling;
  StringRef NewSpelling;
};
}

using RenameHandler =
    internal::RenameHandlerImpl<DeclRefExprNode, NamedDeclNode>;
*/

struct SymbolData {
  SymbolData(std::string File, unsigned Line, unsigned Column,
             std::string NewSpelling)
      : File(std::move(File)), Line(Line), Column(Column),
        NewSpelling(std::move(NewSpelling)) {}

  std::string File;
  unsigned Line;
  unsigned Column;
  std::string NewSpelling;

  Optional<SourceLocation> Loc;
  std::string USR;
  std::string Spelling;
};

template <typename AnnotatedNode>
class RenameHandler : public MatchFinder::MatchCallback {
public:
  RenameHandler(Replacements *Replace, const SymbolData *Data)
      : Replace(Replace), Data(Data) {}

  void run(const MatchFinder::MatchResult &Result) override {
    if (const auto Node =
            Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
                AnnotatedNode::ID())) {
      Replace->insert(Replacement(*(Result.SourceManager), Node->getLocation(),
                                  Data->Spelling.size(), Data->NewSpelling));
    }
  }

private:
  Replacements *Replace;
  const SymbolData *Data;
};

template <typename AnnotatedNode>
class SourceLocationHandler : public MatchFinder::MatchCallback {
public:
  SourceLocationHandler(SymbolData *Data) : Data(Data) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    const auto SourceMgr = Result.SourceManager;
    if (SourceMgr == nullptr)
      return;
    const auto Node = Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
        AnnotatedNode::ID());
    if (Node == nullptr)
      return;
    const NamedDecl *Decl = AnnotatedNode::getNamedDecl(Node);
    if (Decl == nullptr)
      return;
    check(*SourceMgr, Decl, Node->getLocation());
  }

private:
  void check(const SourceManager &SourceMgr, const NamedDecl *Decl,
             const SourceLocation &Start) {
    const auto Length = Decl->getNameAsString().length();
    if (Length == 0)
      return;
    const auto End = Start.getLocWithOffset(Length - 1);
    // If the location is in an expanded macro, we do not want to rename it.
    if (!Start.isValid() || !End.isValid() || Start.isMacroID() ||
        End.isMacroID() || !isLocWithin(SourceMgr, Start, End))
      return;
    Data->USR = getUSRForDecl(*Decl);
    Data->Spelling = Decl->getNameAsString();
  }

  bool isLocWithin(const SourceManager &SourceMgr, const SourceLocation &Start,
                   const SourceLocation &End) {
    if (!Data->Loc.hasValue()) {
      auto &FileMgr = SourceMgr.getFileManager();
      Data->Loc = SourceMgr.translateFileLineCol(FileMgr.getFile(Data->File),
                                                 Data->Line, Data->Column);
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
};
}

// Options
llvm::cl::OptionCategory RenameCategory("rn options");

static llvm::cl::opt<std::string>
    NewSpelling("new-name",
                llvm::cl::desc("The new name to change the symbol to."),
                llvm::cl::cat(RenameCategory), llvm::cl::Required);

static llvm::cl::opt<unsigned>
    Line("line", llvm::cl::desc("The line the symbol is located on."),
         llvm::cl::cat(RenameCategory), llvm::cl::Required);

static llvm::cl::opt<unsigned>
    Column("column", llvm::cl::desc("The column the symbol is located in."),
           llvm::cl::cat(RenameCategory), llvm::cl::Required);

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

  SymbolData Data(Files.front(), Line, Column, NewSpelling);

  tooling::RefactoringTool Tool(OP.getCompilations(), Files);

  {
    SourceLocationHandler<DeclRefExprNode> DeclRefExprHandler(&Data);
    SourceLocationHandler<NamedDeclNode> NamedDeclHandler(&Data);
    MatchFinder Finder;
    Finder.addMatcher(DeclRefExprNode::matchNode(), &DeclRefExprHandler);
    Finder.addMatcher(NamedDeclNode::matchNode(), &NamedDeclHandler);
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to find symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
      exit(1);
    }

    errs() << "USR: " << Data.USR << "\n";
    errs() << "Spelling: " << Data.Spelling << "\n";
  }
  if (Data.USR.empty()) {
    errs() << "Unable to determine USR.";
    exit(1);
  }

  // Find all references and rename them
  {
    auto Replace = &Tool.getReplacements();
    RenameHandler<DeclRefExprNode> DeclRefExprHandler(Replace, &Data);
    RenameHandler<NamedDeclNode> NamedDeclHandler(Replace, &Data);
    MatchFinder Finder;
    Finder.addMatcher(DeclRefExprNode::matchNamedDecl(sameUSR(Data.USR)),
                      &DeclRefExprHandler);
    Finder.addMatcher(NamedDeclNode::matchNamedDecl(sameUSR(Data.USR)),
                      &NamedDeclHandler);
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to rename symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
    }
  }
  llvm::outs() << "Replacements collected by the tool:\n";
  for (auto &r : Tool.getReplacements()) {
    llvm::outs() << r.toString() << "\n";
  }

  return 0;
}
