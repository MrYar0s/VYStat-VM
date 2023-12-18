#ifndef FRONTEND_INCLUDE_SHRIMP_PARSER_HPP
#define FRONTEND_INCLUDE_SHRIMP_PARSER_HPP

#include <memory>
#include <shrimp/frontend/astnode.hpp>
#include <shrimp/lexer.hpp>
#include <iostream>
#include <unordered_map>
#include "shrimp/common/types.hpp"

enum class STATUS : uint8_t { SUCCESS, END, FAIL };

using TokensIter = std::vector<Token>::const_iterator;
using AstRet = std::pair<std::unique_ptr<ASTNode>, enum STATUS>;

class Parser {
public:
    Parser() = default;
    void run(std::vector<Token> &&tokens, std::unique_ptr<ASTNode> &&root);

    AstRet &&getRoot()
    {
        return std::move(root_);
    }

private:
    template <TokenType TOKEN_TYPE>
    bool inline term(std::string *str = nullptr)
    {
        std::cout << "Got token_iter_->value = " << token_iter_->value << std::endl;
        auto prev_iter = token_iter_++;
        bool ret = prev_iter->type == TOKEN_TYPE;
        if (str != nullptr) {
            *str = prev_iter->value;
        }
        return ret;
    }

    AstRet ifStmt(AstRet &&head);
    AstRet retStmt(AstRet &&head);
    AstRet stmtsDeclDash(AstRet &&head);

    AstRet expression(AstRet &&head);
    AstRet simple(AstRet &&head);
    AstRet summand(AstRet &&head);
    AstRet factor(AstRet &&head);
    AstRet primary(AstRet &&head);
    AstRet value(AstRet &&head, ValueType type = ValueType::FLOAT, bool need_to_init = false);

    AstRet functionCall(AstRet &&head);
    AstRet intrinsicCall(AstRet &&head);

    ASTNode::IntrinsicType parseIntrinsicType(std::string &name);

    AstRet expressionDash(AstRet &&head);
    AstRet simpleDash(AstRet &&head);
    AstRet factorDash(AstRet &&head);

    STATUS programDecl();
    STATUS funcDecl();

    AstRet varDecl(AstRet &&head);
    AstRet stmtsDecl(AstRet &&head);

    STATUS programDecl1();
    STATUS programDecl2();

    std::vector<std::pair<std::string, ValueType>> funcParamDecl();
    std::vector<std::string> funcParamCall();

private:
    std::vector<Token> tokens_ {};
    TokensIter token_iter_ {};
    TokensIter reserved_token_iter_ {};

    shrimp::R8Id num_of_tmp_regs_;

    std::unordered_map<std::string, ValueType> ident_to_type;

    AstRet root_;
};

#endif  // FRONTEND_INCLUDE_SHRIMP_PARSER_HPP
