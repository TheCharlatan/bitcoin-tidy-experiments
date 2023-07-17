// Copyright (c) 2023 TheCharlatan
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IncludeCheck.h"

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/Preprocessor.h>

#include <cstdio>

namespace {
} // namespace

namespace bitcoin {


void IncludeCheck::registerPPCallbacks(const clang::SourceManager &SM, clang::Preprocessor *PP, clang::Preprocessor *ModuleExpanderPP) {
  PP->addPPCallbacks(std::make_unique<IncludeCheckPPCallbacks>(*this, SM));
}

IncludeCheck::IncludeCheckPPCallbacks::IncludeCheckPPCallbacks(clang::tidy::ClangTidyCheck &Check, const clang::SourceManager &SM)
  : Check(Check), SM(SM) {}

void IncludeCheck::IncludeCheckPPCallbacks::InclusionDirective(
  clang::SourceLocation HashLoc, const clang::Token &IncludeTok, llvm::StringRef FileName, bool IsAngled,
  clang::CharSourceRange FilenameRange, const clang::FileEntry *File, llvm::StringRef SearchPath,
  llvm::StringRef RelativePath, const clang::Module *Imported, clang::SrcMgr::CharacteristicKind FileType) {

  if (FileName.equals("filesystem")) {
    Check.diag(HashLoc, "Including <filesystem> is prohibited.");
  }
}

} // namespace bitcoin
