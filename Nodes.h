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
  Finder.addMatcher(::rn::Type##Node::matchNode(::rn::sameUSR(Data.USR)),      \
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

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node;
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocation();
  }

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
    return namedDecl(InnerMatcher).bind(ID());
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

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
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

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
    return cxxConstructorDecl(ofClass(InnerMatcher)).bind(ID());
  }
};
/*
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

  static const MatcherType matchNode(const Matcher<clang::NamedDecl> &
InnerMatcher = anything()) { return
loc(tagType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID());
 }
};
*/
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
    if (const auto Type = NestedNameSpecifier->getAsType())
      return Type->getAsTagDecl();
    return nullptr;
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getLocalBeginLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
    return loc(nestedNameSpecifier(anyOf(specifiesType(tagType(hasDeclaration(
                                             namedDecl(InnerMatcher)))),
                                         specifiesNamespace(InnerMatcher))))
        .bind(ID());
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

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
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

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
    return usingDecl(hasAnyUsingShadowDecl(hasTargetDecl(InnerMatcher)))
        .bind(ID());
  }
};

struct AliasedNamespaceNode {
  using NodeType = ::clang::NamespaceAliasDecl;
  using MatcherType = DeclarationMatcher;
  static constexpr const char *ID() { return "AliasedNamespace"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    return Node->getAliasedNamespace();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getTargetNameLoc();
  }

  static const MatcherType
  matchNode(const Matcher<clang::NamedDecl> &InnerMatcher = anything()) {
    return namespaceAliasDecl(aliasesNamespace(InnerMatcher)).bind(ID());
  }
};

/*
struct TypedefTypeNode {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TypedefType"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto TypedefType =
        llvm::dyn_cast_or_null<clang::TypedefType>(Node->getTypePtr());
    if (TypedefType == nullptr)
      return nullptr;
    return TypedefType->getDecl();
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType matchNode(const Matcher<clang::NamedDecl> &
InnerMatcher = anything()) { return
loc(typedefType(hasDeclaration(namedDecl(InnerMatcher)))).bind(ID()); }
};
*/
struct TypeWithDeclarationNode {
  using NodeType = ::clang::TypeLoc;
  using MatcherType = TypeLocMatcher;
  static constexpr const char *ID() { return "TypeWithDeclaration"; }

  static const ::clang::NamedDecl *getNamedDecl(const NodeType *Node) {
    const auto Type = Node->getTypePtr();
    if (Type == nullptr)
      return nullptr;
    if (const auto TagType = llvm::dyn_cast<clang::TagType>(Type))
      return TagType->getDecl();
    if (const auto TypedefType = llvm::dyn_cast<clang::TypedefType>(Type))
      return TypedefType->getDecl();
    if (const auto TemplateTypeParmType =
            llvm::dyn_cast<clang::TemplateTypeParmType>(Type))
      return TemplateTypeParmType->getDecl();
    if (const auto TemplateTypeParmType =
            llvm::dyn_cast<clang::TemplateTypeParmType>(Type))
      return TemplateTypeParmType->getDecl();
    if (const auto TemplateSpecializationType =
            llvm::dyn_cast<clang::TemplateSpecializationType>(Type))
      return TemplateSpecializationType->getTemplateName().getAsTemplateDecl();
    return nullptr;
  }

  static ::clang::SourceLocation getLocation(const NodeType *Node) {
    return Node->getBeginLoc();
  }

  static const MatcherType
  matchNode(const Matcher<::clang::NamedDecl> &InnerMatcher = anything()) {
    const auto DeclMatcher = hasDeclaration(namedDecl(InnerMatcher));
    return loc(type(anyOf(tagType(DeclMatcher), typedefType(DeclMatcher),
                          templateTypeParmType(DeclMatcher),
                          templateSpecializationType(DeclMatcher))))
        .bind(ID());
  }
};
}
