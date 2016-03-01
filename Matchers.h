#pragma once

#include "utility.h"

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <llvm/Support/Casting.h>

#include <string>

namespace rn {

// This matcher matches all NamedDecl's that have the given USR (should only be
// one)
AST_MATCHER_P(clang::NamedDecl, sameUSR, std::string, USR) {
  return getUSRForDecl(&Node) == USR;
}

// Cant use `AST_TYPE_MATCHER(clang::TagType, tagType);` because bad namespacing
const clang::ast_matchers::internal::VariadicDynCastAllOfMatcher<
    clang::Type, clang::TagType> tagType;

const clang::ast_matchers::internal::VariadicDynCastAllOfMatcher<
    clang::Decl, clang::TagDecl> tagDecl;

AST_MATCHER_P(clang::UsingDirectiveDecl, nominatedNamespace,
              clang::ast_matchers::internal::Matcher<clang::NamedDecl>,
              InnerMatcher) {
  const clang::NamedDecl *const NamespaceAsWritten =
      Node.getNominatedNamespaceAsWritten();
  return (NamespaceAsWritten != nullptr &&
          InnerMatcher.matches(*NamespaceAsWritten, Finder, Builder));
}

AST_MATCHER_P(clang::NamespaceAliasDecl, aliasesNamespace,
              clang::ast_matchers::internal::Matcher<clang::NamedDecl>,
              InnerMatcher) {
  const auto NamespaceAsWritten = Node.getAliasedNamespace();
  return (NamespaceAsWritten != nullptr &&
          InnerMatcher.matches(*NamespaceAsWritten, Finder, Builder));
}
}
