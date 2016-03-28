#pragma once

#include "Matchers.h"
#include "Utility.h"

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>

namespace rn {

#define RN_ADD_SOURCE_LOCATION_MATCHER(Type)                                   \
  ::rn::SourceLocationHandler<::rn::Type##Node> Type##Handler(&Data);          \
  Finder.addMatcher(                                                           \
      ::rn::Type##Node::matchNode(::clang::ast_matchers::namedDecl().bind(     \
          ::rn::declID(::rn::Type##Node::ID()))),                              \
      &Type##Handler)

#define RN_ADD_RENAME_MATCHER(Type)                                            \
  ::rn::RenameHandler<::rn::Type##Node> Type##Handler(Replace, &Data);         \
  Finder.addMatcher(                                                           \
      ::rn::Type##Node::matchNode(                                             \
          ::clang::ast_matchers::namedDecl(::rn::sameUSR(Data.USR))            \
              .bind(::rn::declID(::rn::Type##Node::ID()))),                    \
      &Type##Handler)

#define RN_ADD_ALL_MATCHERS(ADD_MATCHER)                                       \
  ADD_MATCHER(NamedDecl);                                                      \
  ADD_MATCHER(DeclRefExpr);                                                    \
  ADD_MATCHER(CXXConstructorDecl);                                             \
  ADD_MATCHER(UsingDirectiveDecl);                                             \
  ADD_MATCHER(UsingDecl);                                                      \
  ADD_MATCHER(AliasedNamespace);                                               \
  ADD_MATCHER(NestedNameSpecifier);                                            \
  ADD_MATCHER(TypeWithDeclaration);                                            \
  ADD_MATCHER(MemberExpr);                                                     \
  ADD_MATCHER(ParmVarDecl);                                                    \
  ; /* Just for easy copy/pasting */

using namespace ::clang::ast_matchers;
using namespace ::clang::ast_matchers::internal;

struct Node {
  template <typename Node>
  static ::llvm::StringRef getSpelling(const Node *,
                                       const ::clang::NamedDecl *Decl) {
    return Decl->getNameAsString();
  }
};

struct NamedDeclNode : Node {
  using NodeType = ::clang::NamedDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "NamedDecl"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return namedDecl(InnerMatcher).bind(ID());
  }
};

struct DeclRefExprNode : Node {
  using NodeType = ::clang::DeclRefExpr;
  using MatcherType = StatementMatcher;
  static constexpr const char *ID() { return "DeclRefExpr"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return declRefExpr(hasDeclaration(InnerMatcher)).bind(ID());
  }
};

struct CXXConstructorDeclNode : Node {
  using NodeType = ::clang::CXXConstructorDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "CXXConstructorDecl"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return cxxConstructorDecl(ofClass(InnerMatcher)).bind(ID());
  }
};

struct NestedNameSpecifierNode : Node {
  using NodeType = ::clang::NestedNameSpecifierLoc;
  using MatcherType = NestedNameSpecifierLocMatcher;
  static constexpr const char *ID() { return "NestedNameSpecifier"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocalBeginLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return loc(nestedNameSpecifier(
                   anyOf(specifiesType(tagType(hasDeclaration(InnerMatcher))),
                         specifiesNamespace(InnerMatcher))))
        .bind(ID());
  }
};

struct UsingDirectiveDeclNode : Node {
  using NodeType = ::clang::UsingDirectiveDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDirectiveDecl"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getIdentLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return usingDirectiveDecl(nominatedNamespace(InnerMatcher)).bind(ID());
  }
};

struct UsingDeclNode : Node {
  using NodeType = ::clang::UsingDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDecl"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getNameInfo().getLoc();
  }

  // TODO: Fail if more than one decl
  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return usingDecl(hasSingleUsingShadowDecl(hasTargetDecl(InnerMatcher)))
        .bind(ID());
  }
};

struct AliasedNamespaceNode : Node {
  using NodeType = ::clang::NamespaceAliasDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "AliasedNamespace"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getTargetNameLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return namespaceAliasDecl(aliasesNamespace(InnerMatcher)).bind(ID());
  }
};

struct TypeWithDeclarationNode : Node {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TypeWithDeclaration"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    const auto DeclMatcher = hasDeclaration(InnerMatcher);
    return loc(type(anyOf(tagType(DeclMatcher), typedefType(DeclMatcher),
                          templateTypeParmType(DeclMatcher),
                          templateSpecializationType(DeclMatcher))))
        .bind(ID());
  }
};

struct MemberExprNode : Node {
  using NodeType = ::clang::MemberExpr;
  using MatcherType = StatementMatcher;
  static constexpr const char *ID() { return "MemberExpr"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getMemberLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return memberExpr(hasDeclaration(InnerMatcher)).bind(ID());
  }
};

struct ParmVarDeclNode : Node {
  enum class Options { One, All, Add };

  static Options RenameOpt;

  using NodeType = ::clang::ParmVarDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "ParmVarDecl"; }

  static ::llvm::StringRef getSpelling(const NodeType *Node,
                                       const ::clang::NamedDecl *) {
    return Node->getNameAsString();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    switch (RenameOpt) {
    case Options::One:
      return parmVarDecl(bestParmVarDecl(InnerMatcher)).bind(ID());
    case Options::All:
      return parmVarDecl(bestParmVarDecl(InnerMatcher)).bind(ID());
    case Options::Add:
      return parmVarDecl(bestParmVarDecl(InnerMatcher)).bind(ID());
    };
  }
};
}
