#include "Handlers.h"
#include "Matchers.h"
#include "Nodes.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/raw_ostream.h>

using clang::tooling::CommonOptionsParser;
using clang::tooling::RefactoringTool;
using clang::tooling::newFrontendActionFactory;

using clang::ast_matchers::MatchFinder;

using llvm::errs;
using llvm::outs;

// Options
llvm::cl::OptionCategory RenameCategory("rn options");

static llvm::cl::opt<std::string>
    NewSpelling("new-name",
                llvm::cl::desc("The new name to change the symbol to."),
                llvm::cl::cat(RenameCategory), llvm::cl::Required);

static llvm::cl::opt<unsigned>
    Line("line", llvm::cl::desc("The line the symbol is located on."),
         llvm::cl::cat(RenameCategory), llvm::cl::Required);

static llvm::cl::opt<unsigned>
    Column("column", llvm::cl::desc("The column the symbol is located in."),
           llvm::cl::cat(RenameCategory), llvm::cl::Required);

const std::string CLANG_RENAME_VERSION = "0.0.1";

static void PrintVersion() {
  outs() << "clang-rename version " << CLANG_RENAME_VERSION << "\n";
}

const char RenameUsage[] = "A tool to rename symbols in C/C++ code.\n\
                            rn renames every occurrence of a symbol found at\
                           < offset >\
                           in\n<source>.The results are written to stdout.\n ";

int main(int argc, const char **argv) {
  using namespace rn;

  llvm::cl::SetVersionPrinter(PrintVersion);
  CommonOptionsParser OP(argc, argv, RenameCategory, RenameUsage);

  if (NewSpelling.empty()) {
    errs() << "rn: no new name provided.\n\n";
    llvm::cl::PrintHelpMessage();
    exit(1);
  }

  auto Files = OP.getSourcePathList();
  if (Files.empty()) {
    errs() << "rn: no files provided.\n\n";
    llvm::cl::PrintHelpMessage();
    exit(1);
  }

  SymbolData Data(Files.front(), Line, Column, NewSpelling);

  RefactoringTool Tool(OP.getCompilations(), Files);

  // Find the source location
  {
    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_SOURCE_LOCATION_MATCHER)
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to find symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
      exit(1);
    }

    errs() << "USR: " << Data.USR << "\n";
    errs() << "Spelling: " << Data.Spelling << "\n";
  }
  if (Data.USR.empty()) {
    errs() << "Unable to determine USR.\n";
    exit(1);
  }

  // Find all references and rename them
  {
    auto Replace = &Tool.getReplacements();

    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_RENAME_MATCHER)
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to rename symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
    }
  }
  llvm::outs() << "Replacements collected by the tool:\n";
  for (auto &r : Tool.getReplacements()) {
    llvm::outs() << r.toString() << "\n";
  }

  return 0;
}
