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

#include <array>
#include <type_traits>
#include <utility>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace clang::tooling;

using llvm::errs;
using llvm::outs;
using llvm::StringRef;

namespace rn {

// Get the USR (a globally unique string) for a NamedDecl
std::string getUSRForDecl(const NamedDecl &Decl) {
  llvm::SmallVector<char, 128> Buf;

  if (index::generateUSRForDecl(&Decl, Buf))
    return std::string{};

  return std::string(Buf.data(), Buf.size());
}

// This matcher matches all NamedDecl's that have the given USR (should only be
// one)
AST_MATCHER_P(NamedDecl, sameUSR, std::string, USR) {
  return getUSRForDecl(Node) == USR;
}

AST_TYPE_MATCHER(clang::TagType, tagType);

const VariadicDynCastAllOfMatcher<clang::Decl, clang::TypeDecl> typeDecl;

AST_MATCHER_P(UsingDirectiveDecl, nominatedNamespace, Matcher<NamespaceDecl>,
              InnerMatcher) {
  const NamespaceDecl *const Namespace = Node.getNominatedNamespace();
  return (Namespace != nullptr &&
          InnerMatcher.matches(*Namespace, Finder, Builder));
}

namespace internal {
struct NamedDeclNode {
  // NodeType is the type of the corresponding node in clang's AST
  using NodeType = clang::NamedDecl;
  // MatcherType is of a matcher that matches NodeType
  using MatcherType = DeclarationMatcher;
  // Return an identifier to use for binding
  static constexpr const char *ID() { return "NamedDecl"; }

  // Takes NodeType and returns the NamedDecl that declares it
  static const NamedDecl *getNamedDecl(const NodeType *Node) { return Node; }

  // Takes a NodeType and returns its source location
  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  // A matcher that matches all NodeType's and binds to ID()
  static const MatcherType matchNode() { return namedDecl().bind(ID()); }

  // A matcher that matches all NodeType's whose NamedDecl matches InnerMatcher
  // and binds to ID()
  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return namedDecl(InnerMatcher).bind(ID());
  }
};
}

struct ValueDeclNode : public internal::NamedDeclNode {
  static constexpr const char *ID() { return "ValueDecl"; }

  static const MatcherType matchNode() { return valueDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return valueDecl(InnerMatcher).bind(ID());
  }
};
/*
struct RecordDeclNode : public internal::NamedDeclNode {
  static constexpr const char *ID() { return "RecordDecl"; }

  static const MatcherType matchNode() { return recordDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return recordDecl(InnerMatcher).bind(ID());
  }
};

struct EnumDeclNode : public internal::NamedDeclNode {
  static constexpr const char *ID() { return "EnumDecl"; }

  static const MatcherType matchNode() { return enumDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return enumDecl(InnerMatcher).bind(ID());
  }
};
*/

struct TypeDeclNode : public internal::NamedDeclNode {
  static constexpr const char *ID() { return "TypeDecl"; }

  static const MatcherType matchNode() { return typeDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return typeDecl(InnerMatcher).bind(ID());
  }
};

struct DeclRefExprNode {
  using NodeType = clang::DeclRefExpr;
  using MatcherType = StatementMatcher;
  static constexpr const char *ID() { return "DeclRefExpr"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getFoundDecl();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() { return declRefExpr().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return declRefExpr(hasDeclaration(namedDecl(InnerMatcher))).bind(ID());
  }
};

struct CXXConstructorDeclNode {
  using NodeType = clang::CXXConstructorDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "CXXConstructorDecl"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getParent();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() {
    return cxxConstructorDecl().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return cxxConstructorDecl(ofClass(InnerMatcher)).bind(ID());
  }
};
/*
struct RecordTypeNode {
  using NodeType = clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "RecordType"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto Type = Node->getType().getTypePtrOrNull();
    if (Type == nullptr)
      return nullptr;
    return Type->getAsCXXRecordDecl();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode() { return loc(recordType()).bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return loc(recordType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID());
  }
};

struct EnumTypeNode {
  using NodeType = clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "EnumType"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto Type = Node->getType().getTypePtrOrNull();
    if (Type == nullptr)
      return nullptr;
    return Type->getAsTagDecl();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode() { return loc(enumType()).bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return loc(enumType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID());
  }
};
*/
struct TagTypeNode {
  using NodeType = clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TagType"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto Type = Node->getType().getTypePtrOrNull();
    if (Type == nullptr)
      return nullptr;
    return Type->getAsTagDecl();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode() { return loc(tagType()).bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return loc(tagType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID());
  }
};

struct NestedNameSpecifierNode {
  using NodeType = clang::NestedNameSpecifierLoc;
  using MatcherType = NestedNameSpecifierLocMatcher;
  static constexpr const char *ID() { return "NestedNameSpecifier"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto NestedNameSpecifier = Node->getNestedNameSpecifier();
    if (NestedNameSpecifier == nullptr)
      return nullptr;
    if (const auto NamespaceDecl = NestedNameSpecifier->getAsNamespace())
      return NamespaceDecl;
    if (const auto NamespaceAliasDecl =
            NestedNameSpecifier->getAsNamespaceAlias())
      return NamespaceAliasDecl;
    return nullptr;
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocalBeginLoc();
  }

  static const MatcherType matchNode() {
    return nestedNameSpecifierLoc().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return loc(specifiesNamespace(InnerMatcher)).bind(ID());
  }
};

struct UsingDirectiveDeclNode {
  using NodeType = clang::UsingDirectiveDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDirectiveDecl"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getNominatedNamespaceAsWritten();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getIdentLocation();
  }

  static const MatcherType matchNode() {
    return usingDirectiveDecl().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return usingDirectiveDecl(nominatedNamespace(InnerMatcher)).bind(ID());
  }
};

struct UsingDeclNode {
  using NodeType = clang::UsingDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDecl"; }

  static const NamedDecl *getNamedDecl(const NodeType *Node) {
    // If this UsingDecl shadows more than one declaration, e.g. an overloaded
    // function, then we give up on it since there is no way to know what to
    // rename.
    if (Node->shadow_size() != 1)
      return nullptr;
    const auto UsingShadowDecl = *(Node->shadow_begin());
    if (UsingShadowDecl == nullptr)
      return nullptr;
    return UsingShadowDecl->getTargetDecl();
  }

  static SourceLocation getLocation(const NodeType *Node) {
    return Node->getNameInfo().getLoc();
  }

  static const MatcherType matchNode() { return usingDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<NamedDecl> &InnerMatcher) {
    return usingDecl(hasAnyUsingShadowDecl(hasTargetDecl(InnerMatcher)))
        .bind(ID());
  }
};

#define RN_ADD_SOURCE_LOCATION_MATCHER(Type)                                   \
  ::rn::SourceLocationHandler<::rn::Type##Node> Type##Handler(&Data);          \
  Finder.addMatcher(::rn::Type##Node::matchNode(), &Type##Handler)

#define RN_ADD_RENAME_MATCHER(Type)                                            \
  ::rn::RenameHandler<::rn::Type##Node> Type##Handler(Replace, &Data);         \
  Finder.addMatcher(::rn::Type##Node::matchNamedDecl(::rn::sameUSR(Data.USR)), \
                    &Type##Handler)

#define RN_ADD_ALL_MATCHERS(ADD_MATCHER)                                       \
  ADD_MATCHER(DeclRefExpr);                                                    \
  ADD_MATCHER(ValueDecl);                                                      \
  ADD_MATCHER(TypeDecl);                                                       \
  ADD_MATCHER(CXXConstructorDecl);                                             \
  ADD_MATCHER(TagType);                                                        \
  ADD_MATCHER(NestedNameSpecifier);                                            \
  ADD_MATCHER(UsingDirectiveDecl);                                             \
  ADD_MATCHER(UsingDecl);                                                      \
  ; /* Just for easy copy/pasting */

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
    if (std::is_same<AnnotatedNode, UsingDeclNode>::value) {
      errs() << "Matched a UsingDecl\n";
    }
    // Rename the Node if there is a match
    if (const auto Node =
            Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
                AnnotatedNode::ID())) {
      Replace->insert(Replacement(*(Result.SourceManager),
                                  AnnotatedNode::getLocation(Node),
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
    // Get the matched node
    const auto Node = Result.Nodes.getNodeAs<typename AnnotatedNode::NodeType>(
        AnnotatedNode::ID());
    if (Node == nullptr)
      return;
    // and its NamedDecl
    const NamedDecl *Decl = AnnotatedNode::getNamedDecl(Node);
    if (Decl == nullptr)
      return;
    // See if it is at the location we are looking for
    check(*SourceMgr, Decl, AnnotatedNode::getLocation(Node));
  }

private:
  void check(const SourceManager &SourceMgr, const NamedDecl *Decl,
             const SourceLocation &Start) {
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
    Data->USR = getUSRForDecl(*Decl);
    Data->Spelling = Decl->getNameAsString();
  }

  // Returns true if Data->Loc is within [Start, End].
  bool isLocWithin(const SourceManager &SourceMgr, const SourceLocation &Start,
                   const SourceLocation &End) {
    // We only have a SourceManager in the callback, so if the Loc hasn't been
    // translated from <file>:<line>:<col> yet, translate it and save the data.
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
/*
template <unsigned I, unsigned N, template <typename> class Handler,
          typename... AnnotatedNodes>
struct AddMatchersImpl;

template <unsigned I, unsigned N, template <typename> class Handler>
struct AddMatchersImpl<I, N, Handler> {
  template <typename Fn, typename... Args>
  void operator()(const Fn &MatchAdder, MatchFinder &Finder,
                  std::array<MatchFinder::MatchCallback, N> &Handlers,
                  Args &&... args) const {}
};

template <unsigned I, unsigned N, template <typename> class Handler,
          typename AnnotatedNode, typename... Rest>
struct AddMatchersImpl<I, N, Handler, AnnotatedNode, Rest...> {
  template <typename Fn, typename... Args>
  void operator()(const Fn &MatchAdder, MatchFinder &Finder,
                  std::array<MatchFinder::MatchCallback, N> &Handlers,
                  Args &&... args) const {
    Handlers[I] = Handler<AnnotatedNode>{std::forward<Args>(args)...};
    // MatchAdder.template run<AnnotatedNode>(Finder, &Handlers[I]);
    Finder.addMatcher(MatchAdder.template run<AnnotatedNode>(), &Handlers[]
  });
    AddMatchersImpl<I + 1, N, Handler, Rest...>{}(MatchAdder, Finder, Handlers,
                                                  std::forward<Args>(args)...);
  }
};

template <template <typename> class Handler, typename... AnnotatedNodes>
struct AddMatchers {
  template <typename Fn, typename... Args>
  std::array<MatchFinder::MatchCallback, sizeof...(AnnotatedNodes)>
  operator()(const Fn &MatchAdder, MatchFinder &Finder, Args &&... args) {
    std::array<MatchFinder::MatchCallback, sizeof...(AnnotatedNodes)> Handlers;
    AddMatchersImpl<0, sizeof...(AnnotatedNodes), Handler,
                    AnnotatedNodes...>{}(MatchAdder, Finder, Handlers,
                                         std::forward<Args>(args)...);
    return Handlers;
  }
};

struct SourceLocMatcher {
  template <typename AnnotatedNode>
  typename AnnotatedNode::MatcherType run() const {
    return AnnotatedNode::matchNode();
  }
};

struct RenameMatcher {
  RenameMatcher(std::string USR) : USR(std::move(USR)) {}
  template <typename AnnotatedNode>
  typename AnnotatedNode::MatcherType run() const {
    return AnnotatedNode::matchNamedDecl(sameUSR(USR));
  }

private:
  std::string USR;
};
*/
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
                            rn renames every occurrence of a symbol found at\
                           < offset >\
                           in\n<source>.The results are written to stdout.\n ";

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

  // Find the source location
  {
    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_SOURCE_LOCATION_MATCHER)
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to find symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
      exit(1);
    }

    errs() << "USR: " << Data.USR << "\n";
    errs() << "Spelling: " << Data.Spelling << "\n";
  }
  if (Data.USR.empty()) {
    errs() << "Unable to determine USR.\n";
    exit(1);
  }

  // Find all references and rename them
  {
    auto Replace = &Tool.getReplacements();

    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_RENAME_MATCHER)
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

#undef RN_ADD_SOURCE_LOCATION_MATCHER

#undef RN_ADD_RENAME_MATCHER

#undef RN_ADD_ALL_MATCHERS
