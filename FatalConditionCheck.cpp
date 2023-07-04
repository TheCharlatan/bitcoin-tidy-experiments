// Copyright (c) 2023 TheCharlatan
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "FatalConditionCheck.h"

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace {
AST_MATCHER_P2(clang::FunctionDecl, returnsResultOfFatalCondition,
               std::string, result_type,
               std::string, error_type) {
    // Check return type
    const auto* return_type = Node.getReturnType().getTypePtr();
    if (!return_type->isInstantiationDependentType() &&
        return_type->getAs<clang::TemplateSpecializationType>()) {
        const auto* spec_type = return_type->getAs<clang::TemplateSpecializationType>();

        // Check if return type is 'util::Result'
        if (spec_type->getTemplateName().getAsTemplateDecl()->getNameAsString() == result_type) {

            // Check template arguments
            const auto& args = spec_type->template_arguments();
            if (args.size() == 2) { // Assuming Result has exactly two template parameters

                // Get the error type
                if (auto error_type_decl = args[1].getAsType()->getAs<clang::EnumType>()) {
                    if (error_type_decl->getDecl()->getNameAsString() == error_type) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

} // namespace

namespace bitcoin {

void FatalConditionCheck::registerMatchers(clang::ast_matchers::MatchFinder *finder) {
    using namespace clang::ast_matchers;
    finder->addMatcher(
        functionDecl(returnsResultOfFatalCondition("Result", "FatalCondition")).bind("fatal_condition_func"),
        this);

    finder->addMatcher(
        functionDecl(
            unless(returnsResultOfFatalCondition("Result", "FatalCondition")),
            forEachDescendant(
                callExpr(
                    callee(functionDecl(returnsResultOfFatalCondition("Result", "FatalCondition")))
                )
            )
        ).bind("fatal_condition_func_caller"),
        this
    );
}

void FatalConditionCheck::check(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    if (const auto* fatal_condition_func = Result.Nodes.getNodeAs<clang::FunctionDecl>("fatal_condition_func")) {
		auto loc = fatal_condition_func->getLocation();
		// Check if the function has the [[nodiscard]] attribtue
		if (!fatal_condition_func->hasAttr<clang::WarnUnusedResultAttr>()) {
            const auto& ctx = *Result.Context;
            auto loc = fatal_condition_func->getBeginLoc();
            auto diag_builder = diag(loc, "Function returning util::Result<T, FatalCondition> should be marked [[nodiscard]].");
            diag_builder << clang::FixItHint::CreateInsertion(loc, "[[nodiscard]] ");
		}
    }

    const auto* fatal_condition_func_caller = Result.Nodes.getNodeAs<clang::FunctionDecl>("fatal_condition_func_caller");
    if (fatal_condition_func_caller) {
        diag(fatal_condition_func_caller->getBeginLoc(), "Function calls another function returning FatalCondition Result but does not itself return FatalCondition Result.");
    }
}

} // bitcoin

