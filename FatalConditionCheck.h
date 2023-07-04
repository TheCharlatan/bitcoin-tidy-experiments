// Copyright (c) 2022 TheCharlatan
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FATAL_CONDITION_CHECK_H
#define FATAL_CONDITION_CHECK_H

#include <clang-tidy/ClangTidyCheck.h>

namespace bitcoin {

class FatalConditionCheck final : public clang::tidy::ClangTidyCheck {

public:
  FatalConditionCheck(clang::StringRef Name, clang::tidy::ClangTidyContext *Context)
      : clang::tidy::ClangTidyCheck(Name, Context) {}

  bool isLanguageVersionSupported(const clang::LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
  void registerMatchers(clang::ast_matchers::MatchFinder *Finder) override;
  void check(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace bitcoin

#endif // FATAL_CONDITION_CHECK_H
