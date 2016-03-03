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
          ::clang::ast_matchers::namedDecl(::rn::sameUSR(Data.USR))),          \
      &Type##Handler)

#define RN_ADD_ALL_MATCHERS(ADD_MATCHER)                                       \
  ADD_MATCHER(NamedDecl);                                                      \
  ADD_MATCHER(DeclRefExpr);                                                    \
  ADD_MATCHER(CXXConstructorDecl);                                             \
  ADD_MATCHER(UsingDirectiveDecl);                                             \
  ADD_MATCHER(UsingDecl);                                                      \
  ADD_MATCHER(AliasedNamespace);                                               \
  ADD_MATCHER(NestedNameSpecifier);                                            \
  ADD_MATCHER(TypeWithDeclaration);
; /* Just for easy copy/pasting */

using namespace ::clang::ast_matchers;
using namespace ::clang::ast_matchers::internal;

struct NamedDeclNode {
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

struct DeclRefExprNode {
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

struct CXXConstructorDeclNode {
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
/*
struct TagTypeNode {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TagType"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode(const Matcher<clang::Decl> &
InnerMatcher = anything()) { return
loc(tagType(hasDeclaration(InnerMatcher))).bind(ID());
 }
};
*/
struct NestedNameSpecifierNode {
  using NodeType = ::clang::NestedNameSpecifierLoc;
  using MatcherType = NestedNameSpecifierLocMatcher;
  static constexpr const char *ID() { return "NestedNameSpecifier"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocalBeginLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::Decl> &InnerMatcher = anything()) {
    return loc(nestedNameSpecifier(anyOf(specifiesType(tagType(hasDeclaration(
                                             InnerMatcher))),
                                         specifiesNamespace(InnerMatcher))))
        .bind(ID());
  }
};

struct UsingDirectiveDeclNode {
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

struct UsingDeclNode {
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

struct AliasedNamespaceNode {
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

/*
struct TypedefTypeNode {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TypedefType"; }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode(const Matcher<clang::Decl> &
InnerMatcher = anything()) { return
loc(typedefType(hasDeclaration(InnerMatcher))).bind(ID()); }
};
*/
struct TypeWithDeclarationNode {
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
}
