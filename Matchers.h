#pragma once

#include <iostream>
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

AST_MATCHER_P(clang::UsingDecl, hasSingleUsingShadowDecl,
              clang::ast_matchers::internal::Matcher<clang::UsingShadowDecl>,
              InnerMatcher) {
  if (Node.shadow_size() != 1)
    return false;
  return InnerMatcher.matches(**(Node.shadow_begin()), Finder, Builder);
}

AST_MATCHER_P(clang::ParmVarDecl, anySameParmVarDecl,
              clang::ast_matchers::internal::Matcher<clang::ParmVarDecl>,
              InnerMatcher) {
  if (InnerMatcher.matches(Node, Finder, Builder)) {
    return true;
  }
  const auto *Function = llvm::dyn_cast_or_null<clang::FunctionDecl>(
      Node.getParentFunctionOrMethod());
  if (Function == nullptr)
    return false;
  const auto ArgNum = Node.getFunctionScopeIndex();
  for (const auto Redecl : Function->redecls()) {
    const clang::ParmVarDecl *OtherDecl = Redecl->getParamDecl(ArgNum);
    if (OtherDecl != nullptr &&
        InnerMatcher.matches(*OtherDecl, Finder, Builder)) {
      return true;
    }
  }
  return false;
}

AST_MATCHER_P(clang::ParmVarDecl, bestParmVarDecl,
              clang::ast_matchers::internal::Matcher<clang::ParmVarDecl>,
              InnerMatcher) {
  const auto Name = Node.getNameAsString();
  if (Name.empty())
    return false;
  const auto *Function = llvm::dyn_cast_or_null<clang::FunctionDecl>(
      Node.getParentFunctionOrMethod());
  if (Function == nullptr) 
    return InnerMatcher.matches(Node, Finder, Builder);


  // If the function is defined w/ named parameter, this is the best
  const clang::FunctionDecl *FunctionDef = nullptr;
  Function->hasBody(FunctionDef);
  if (FunctionDef != nullptr) {
    if (const clang::ParmVarDecl *OtherDecl =
            FunctionDef->getParamDecl(Node.getFunctionScopeIndex())) {
      if (OtherDecl->getNameAsString() == Name)
        return InnerMatcher.matches(*OtherDecl, Finder, Builder);
    }
  }

  // Otherwise get the first function decl w/ named parameter
  // Note: Can't use Function->redecls(), since order differs depending
  // on which node you call it on.
  const auto *Redecl = Function->getMostRecentDecl();
  while (Redecl != nullptr) {
  if (const clang::ParmVarDecl *OtherDecl =
          Redecl->getParamDecl(Node.getFunctionScopeIndex())) {
    if (OtherDecl->getNameAsString() == Name)
      return InnerMatcher.matches(*OtherDecl, Finder, Builder);
    Redecl = Redecl->getPreviousDecl();
  }
}

return false;
}
}
