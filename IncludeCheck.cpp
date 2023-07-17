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

  clang::FullSourceLoc fullLoc(HashLoc, SM);
  const clang::FileEntry* currentFile = SM.getFileEntryForID(fullLoc.getFileID());

  // Skip the check for the allowed files
  if (currentFile && (
        currentFile->getName().endswith("fs.h")
        || currentFile->getName().endswith("fs_helpers.cpp")
        || currentFile->getName().endswith("fs_helpers.h")
        )) {
    return;
  }

  // Check for the filesystem include
  if (FileName.equals("filesystem")) {
    Check.diag(HashLoc, "Including <filesystem> is prohibited. Don't.");
  }

  // Check if the file contains a string matching with std::filesystem
  clang::FileID fileID = SM.translateFile(currentFile);
  bool Invalid = false;
  llvm::StringRef fileContent = SM.getBufferData(fileID, &Invalid);
  if (!Invalid) {
    if (fileContent.contains("std::filesystem")) {
      clang::SourceLocation fileStartLoc = SM.getLocForStartOfFile(SM.getFileID(HashLoc));
      Check.diag(fileStartLoc, "Using std::filesystem is not allowed. Use util::fs instead.");
    }
  }

}

void IncludeCheck::registerMatchers(clang::ast_matchers::MatchFinder *Finder) {
  // Match on variables with the std::filesystem::path type
  Finder->addMatcher(
      clang::ast_matchers::varDecl(
        clang::ast_matchers::hasType(
          clang::ast_matchers::qualType(
            clang::ast_matchers::hasCanonicalType(
              clang::ast_matchers::recordType(
                clang::ast_matchers::hasDeclaration(
                  clang::ast_matchers::cxxRecordDecl(
                    clang::ast_matchers::hasName("::std::filesystem::path")
                    )
                  )
                )
              )
            )
          )
        ).bind("var_declared_as_std_filesystem_path"),
      this
      );

  // Match on calls to the std::filesystem::path::string method
  Finder->addMatcher(
      clang::ast_matchers::cxxMemberCallExpr(
        clang::ast_matchers::on(clang::ast_matchers::expr(
            clang::ast_matchers::hasType(
              clang::ast_matchers::qualType(
                clang::ast_matchers::hasDeclaration(
                  clang::ast_matchers::cxxRecordDecl(
                    clang::ast_matchers::hasName(
                      "::std::filesystem::path"))))))),
        clang::ast_matchers::callee(clang::ast_matchers::cxxMethodDecl(
            clang::ast_matchers::hasName("string"))))
      .bind("invokes_method_std_filesystem_path_string"),
      this);
}

void IncludeCheck::check(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
  // Raise a diagnostic when std::filesystem::path variables were found
  if (const auto *Var = Result.Nodes.getNodeAs<clang::VarDecl>("var_declared_as_std_filesystem_path")) {
    // Skip this check for certain files
    clang::SourceManager &SM = *Result.SourceManager;
    clang::SourceLocation Loc = Var->getLocation();
    if (Loc.isValid()) {
      llvm::StringRef FileName = SM.getFilename(Loc);
      if (FileName.contains("fs.h") || FileName.contains("fs.cpp")) {
        return;
      }
    }
    diag(Var->getBeginLoc(), "Usage of std::filesystem is prohibited. Use util::fs instead.");
  }

  // Raise a diagnostic when calls std::filesystem::path::string method were found
  if (const auto *Call = Result.Nodes.getNodeAs<clang::CallExpr>("invokes_method_std_filesystem_path_string")) {
    diag(Call->getBeginLoc(), "Usage of std::filesystem::path::string is prohibited. Use util::fs::path::string instead.");
  }
}

} // namespace bitcoin
