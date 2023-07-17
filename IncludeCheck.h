// Copyright (c) 2022 TheCharlatan
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INCLUDE_CHECK_H
#define INCLUDE_CHECK_H

#include <clang-tidy/ClangTidyCheck.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Lex/PPCallbacks.h>

namespace bitcoin {

class IncludeCheck : public clang::tidy::ClangTidyCheck {
public:
  IncludeCheck(llvm::StringRef Name, clang::tidy::ClangTidyContext *Context)
      : clang::tidy::ClangTidyCheck(Name, Context) {}

  void registerPPCallbacks(const clang::SourceManager &SM, clang::Preprocessor *PP, clang::Preprocessor *ModuleExpanderPP) override;

private:
  class IncludeCheckPPCallbacks : public clang::PPCallbacks {
  public:
    IncludeCheckPPCallbacks(clang::tidy::ClangTidyCheck &Check, const clang::SourceManager &SM);

    void InclusionDirective(clang::SourceLocation HashLoc, const clang::Token &IncludeTok, llvm::StringRef FileName,
                            bool IsAngled, clang::CharSourceRange FilenameRange, const clang::FileEntry *File,
                            llvm::StringRef SearchPath, llvm::StringRef RelativePath, const clang::Module *Imported,
                            clang::SrcMgr::CharacteristicKind FileType) override;

  private:
    clang::tidy::ClangTidyCheck &Check;
    const clang::SourceManager &SM;
  };
};

} // namespace bitcoin

#endif // INCLUDE_CHECK_H
