#include "Rename/Nodes.h"
#include "Rename/Options.h"

#include <clang/Tooling/CommonOptionsParser.h>

namespace rn {

llvm::cl::opt<ParmVarDeclNode::Options, true> ParmVarNodeOptions{
    "parm-var-strictness", llvm::cl::desc("The line the symbol is located on."),
    llvm::cl::values(
        clEnumValN(
            ParmVarDeclNode::Options::One, "one",
            "Only rename the variables in this declaration or definition"),
        clEnumValN(ParmVarDeclNode::Options::All, "all",
                   "Rename all the variables in every related "
                   "declaration or definition"),
        clEnumValN(
            ParmVarDeclNode::Options::Add, "add",
            "Same as 'all', but add names to variables that are unnamed"),
        clEnumValEnd),
    llvm::cl::location(ParmVarDeclNode::RenameOpt),
    llvm::cl::init(ParmVarDeclNode::Options::One),
    llvm::cl::cat(RenameCategory)};
}
