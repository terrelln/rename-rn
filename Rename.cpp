#include <Rename/Handlers.h>
#include <Rename/Matchers.h>
#include <Rename/Nodes.h>
#include <Rename/Options.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Refactoring.h>

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/raw_ostream.h>

using clang::IgnoringDiagConsumer;
using clang::tooling::CommonOptionsParser;
using clang::tooling::RefactoringTool;
using clang::tooling::newFrontendActionFactory;

using clang::ast_matchers::MatchFinder;

using llvm::errs;
using llvm::outs;

namespace rn {
llvm::cl::OptionCategory RenameCategory{"rn options"};
// Command line options
static llvm::cl::opt<std::string> NewSpelling{
    "new-name", llvm::cl::desc("The new name to change the symbol to."),
    llvm::cl::cat(RenameCategory), llvm::cl::Required};

static llvm::cl::opt<unsigned>
    Line{"line", llvm::cl::desc("The line the symbol is located on."),
         llvm::cl::cat(RenameCategory), llvm::cl::Required};

static llvm::cl::opt<unsigned>
    Column{"column", llvm::cl::desc("The column the symbol is located in."),
           llvm::cl::cat(RenameCategory), llvm::cl::Required};

static llvm::cl::opt<bool>
    Rewrite{"rewrite", llvm::cl::desc("Should the files be rewritten."),
            llvm::cl::cat(RenameCategory), llvm::cl::Required};

// The tool version to display
const std::string RENAME_RN_VERSION = "0.0.1";

// The function that prints the version
inline void PrintVersion() {
  llvm::outs() << "clang-rename version " << RENAME_RN_VERSION << "\n";
}

const char RenameUsage[] = "A tool to rename symbols in C/C++ code.\n\
                            rn renames every occurrence of a symbol found at\
                           < offset >\
                           in\n<source>.The results are written to stdout.\n ";
}

int main(int argc, const char **argv) {
  using namespace rn;

  llvm::cl::SetVersionPrinter(PrintVersion);
  CommonOptionsParser OP(argc, argv, RenameCategory, RenameUsage);

  if (NewSpelling.empty()) {
    errs() << "rn: no new name provided.\n\n";
    llvm::cl::PrintHelpMessage();
    return 1;
  }

  auto Files = OP.getSourcePathList();
  if (Files.empty()) {
    errs() << "rn: no files provided.\n\n";
    llvm::cl::PrintHelpMessage();
    return 1;
  }

  SymbolData Data(Files.front(), Line, Column, NewSpelling);

  RefactoringTool Tool(OP.getCompilations(), Files);
  IgnoringDiagConsumer DiagConsumer;
  Tool.setDiagnosticConsumer(&DiagConsumer);

  // Find the source location
  {
    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_SOURCE_LOCATION_MATCHER)
    if (Tool.run(newFrontendActionFactory(&Finder).get())) {
      errs() << "Failed to find symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
      return 1;
    }
  }
  if (Data.USR.empty()) {
    errs() << "Unable to determine USR.\n";
    return 1;
  }

  // Find all references and rename them
  {
    auto Replace = &Tool.getReplacements();

    MatchFinder Finder;
    RN_ADD_ALL_MATCHERS(RN_ADD_RENAME_MATCHER)
    bool Result;
    if (Rewrite) {
      Result = Tool.runAndSave(newFrontendActionFactory(&Finder).get());
    } else {
      Result = Tool.run(newFrontendActionFactory(&Finder).get());
    }
    if (Result) {
      errs() << "Failed to rename symbol at location: " << Files.front() << ":"
             << Line << ":" << Column << ".\n";
    }
  }
  if (!Rewrite) {
    llvm::outs() << "Replacements collected by the tool:\n";
    for (auto &r : Tool.getReplacements()) {
      llvm::outs() << r.toString() << "\n";
    }
  }

  return 0;
}
