#pragma once

#include "Matchers.h"

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>

namespace rn {

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

using namespace ::clang::ast_matchers;
using namespace ::clang::ast_matchers::internal;

struct ValueDeclNode {
  using NodeType = ::clang::ValueDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "ValueDecl"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node;
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() { return valueDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return valueDecl(InnerMatcher).bind(ID());
  }
};

struct TypeDeclNode {
  using NodeType = ::clang::NamedDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "TypeDecl"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node;
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() { return typeDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return typeDecl(InnerMatcher).bind(ID());
  }
};

struct DeclRefExprNode {
  using NodeType = ::clang::DeclRefExpr;
  using MatcherType = StatementMatcher;
  static constexpr const char *ID() { return "DeclRefExpr"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getFoundDecl();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() { return declRefExpr().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return declRefExpr(hasDeclaration(namedDecl(InnerMatcher))).bind(ID());
  }
};

struct CXXConstructorDeclNode {
  using NodeType = ::clang::CXXConstructorDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "CXXConstructorDecl"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getParent();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType matchNode() {
    return cxxConstructorDecl().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return cxxConstructorDecl(ofClass(InnerMatcher)).bind(ID());
  }
};

struct TagTypeNode {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TagType"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto Type = Node->getType().getTypePtrOrNull();
    if (Type == nullptr)
      return nullptr;
    return Type->getAsTagDecl();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode() { return loc(tagType()).bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return loc(tagType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID());
  }
};

struct NestedNameSpecifierNode {
  using NodeType = ::clang::NestedNameSpecifierLoc;
  using MatcherType = NestedNameSpecifierLocMatcher;
  static constexpr const char *ID() { return "NestedNameSpecifier"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
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

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocalBeginLoc();
  }

  static const MatcherType matchNode() {
    return nestedNameSpecifierLoc().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return loc(specifiesNamespace(InnerMatcher)).bind(ID());
  }
};

struct UsingDirectiveDeclNode {
  using NodeType = ::clang::UsingDirectiveDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDirectiveDecl"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getNominatedNamespaceAsWritten();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getIdentLocation();
  }

  static const MatcherType matchNode() {
    return usingDirectiveDecl().bind(ID());
  }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return usingDirectiveDecl(nominatedNamespace(InnerMatcher)).bind(ID());
  }
};

struct UsingDeclNode {
  using NodeType = ::clang::UsingDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "UsingDecl"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
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

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getNameInfo().getLoc();
  }

  static const MatcherType matchNode() { return usingDecl().bind(ID()); }

  static const MatcherType
  matchNamedDecl(const Matcher<::clang::NamedDecl> &InnerMatcher) {
    return usingDecl(hasAnyUsingShadowDecl(hasTargetDecl(InnerMatcher)))
        .bind(ID());
  }
};
}
