#pragma once

#include <clang/AST/AST.h>
#include <clang/Index/USRGeneration.h>

#include <llvm/ADT/SmallVector.h>

#include <string>

namespace rn {

// Get the USR (a globally unique string) for a NamedDecl
std::string getUSRForDecl(const clang::NamedDecl *Decl) {
  llvm::SmallVector<char, 128> Buf;

  const auto *CanonicalDecl = Decl->getCanonicalDecl();

  if (CanonicalDecl == nullptr ||
      clang::index::generateUSRForDecl(CanonicalDecl, Buf))
    return std::string{};

  return std::string(Buf.data(), Buf.size());
}
}
