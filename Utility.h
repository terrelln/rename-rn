#pragma once

#include <clang/AST/AST.h>
#include <clang/Index/USRGeneration.h>

#include <llvm/ADT/SmallVector.h>

#include <string>

namespace rn {

static inline std::string declID(std::string ID) { return ID + "Decl"; }

// Get the USR (a globally unique string) for a NamedDecl
static inline std::string getUSRForDecl(const clang::NamedDecl *Decl) {
  llvm::SmallVector<char, 128> Buf;

  const auto *CanonicalDecl = Decl->getCanonicalDecl();

  if (CanonicalDecl == nullptr ||
      clang::index::generateUSRForDecl(CanonicalDecl, Buf))
    return std::string{};

  return std::string(Buf.data(), Buf.size());
}
}
