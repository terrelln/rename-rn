#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/ADT/StringRef.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace clang::tooling;

using llvm::StringRef;

namespace rn {

bool isSameDecl(const Decl &First, const Decl &Second) {
  return First.getCanonicalDecl() == Second.getCanonicalDecl();
}

AST_MATCHER_P(Decl, singularDeclNode1, const Decl *, Declaration) {
  if (Declaration == nullptr)
    return false;
  return isSameDecl(Node, *Declaration);
}

StatementMatcher varDeclRefMatcher(const VarDecl &Decl) {
  return declRefExpr(hasDeclaration(singularDeclNode1(&Decl)));
}

class VarDeclRefHandler : public MatchFinder::MatchCallback {
public:
  VarDeclRefHandler(Replacements *Replace, StringRef NewSpelling)
      : Replace(Replace), NewSpelling(NewSpelling) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const DeclRefExpr *Ref = Result.Nodes.getNodeAs<DeclRefExpr>("expr")) {
      Replacement Rep(*(Result.SourceManager), Ref, NewSpelling);
      Replace->insert(Rep);
    }
  }

private:
  Replacements *Replace;
  StringRef NewSpelling;
};
}
int main() {}
