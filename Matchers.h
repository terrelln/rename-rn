#pragma once

#include "utility.h"

#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <string>

namespace rn {

// This matcher matches all NamedDecl's that have the given USR (should only be
// one)
AST_MATCHER_P(clang::NamedDecl, sameUSR, std::string, USR) {
  return getUSRForDecl(Node) == USR;
}

// Cant use `AST_TYPE_MATCHER(clang::TagType, tagType);` because bad namespacing
const clang::ast_matchers::internal::VariadicDynCastAllOfMatcher<
    clang::Type, clang::TagType> tagType;

const clang::ast_matchers::internal::VariadicDynCastAllOfMatcher<
    clang::Decl, clang::TypeDecl> typeDecl;

AST_MATCHER_P(clang::UsingDirectiveDecl, nominatedNamespace,
              clang::ast_matchers::internal::Matcher<clang::NamespaceDecl>,
              InnerMatcher) {
  const clang::NamespaceDecl *const Namespace = Node.getNominatedNamespace();
  return (Namespace != nullptr &&
          InnerMatcher.matches(*Namespace, Finder, Builder));
}
}
