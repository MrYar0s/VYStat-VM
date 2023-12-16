#include <memory>
#include <shrimp/parser.hpp>
#include <shrimp/frontend/astnode.hpp>
#include <iostream>
#include <shrimp/lexer.hpp>
#include <utility>

void Parser::run(std::vector<Token> &&tokens, std::unique_ptr<ASTNode> &&root)
{
    root_ = AstRet(std::move(root), STATUS::SUCCESS);

    tokens_ = std::move(tokens);

    token_iter_ = tokens_.begin();
    reserved_token_iter_ = token_iter_;

    bool ret = programDecl() != STATUS::FAIL;

    if (!ret) {
        std::cerr << "Wrong parser output" << std::endl;
    }
}

STATUS Parser::programDecl1()
{
    std::cout << "Program: FuncDecl" << std::endl;
    return funcDecl();
}

STATUS Parser::programDecl2()
{
    std::cout << "Program: Program FuncDecl" << std::endl;
    return programDecl();
}

STATUS Parser::programDecl()
{
    STATUS status;
    if ((status = programDecl1()) == STATUS::FAIL || status == STATUS::END) {
        return status;
    }
    if ((status = programDecl2()) == STATUS::FAIL || status == STATUS::END) {
        return status;
    }
    return STATUS::SUCCESS;
}

STATUS Parser::funcDecl()
{
    std::cout << "FuncDecl: FUNC Ident \'(\' FuncParams \')\' \'{\' Statements \'}\'" << std::endl;

    if (term<TokenType::END>()) {
        token_iter_--;
        return STATUS::END;
    }
    token_iter_--;

    std::string ident = "";

    if (!term<TokenType::FUNCTION>()) {
        std::cout << "Missing keyword \"function\"" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::IDENTIFIER>(&ident)) {
        std::cout << "Missing identifier" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!(term<TokenType::OPEN_BRACKET>() && funcParamDecl() && term<TokenType::CLOSE_BRACKET>())) {
        std::cout << "Wrong function arguments" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::OPEN_FIG_BRACKET>()) {
        std::cout << "Expected \'{\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    auto head = AstRet(std::make_unique<ASTNode>(ASTNode::NodeKind::FUNCTION_DECL, ident), STATUS::SUCCESS);

    if ((head = stmtsDecl(std::move(head))).second == STATUS::FAIL) {
        std::cout << "Wrong statements declaration" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::CLOSE_FIG_BRACKET>()) {
        std::cout << "Expected \'}\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    root_.first->AddChildNode(std::move(head.first));

    reserved_token_iter_ = token_iter_;
    return STATUS::SUCCESS;
}

AstRet Parser::stmtsDecl(AstRet &&head)
{
    std::cout << "Statements" << std::endl;
    if ((head = varDecl(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = retStmt(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    token_iter_ = reserved_token_iter_;
    head.second = head.second;
    return head;
}

AstRet Parser::stmtsDeclDash(AstRet &&head)
{
    reserved_token_iter_ = token_iter_;
    if ((head = stmtsDecl(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        return head;
    }
    return head;
}

AstRet Parser::retStmt(AstRet &&head)
{
    std::cout << "ReturnStatement" << std::endl;

    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    if (!term<TokenType::RETURN>()) {
        std::cout << "Expected return keyword" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<RetStmt>(ASTNode::NodeKind::RETURN_STATEMENT);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        std::cout << "Wrong Expression parsing" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::SEMICOLON>()) {
        std::cout << "Expected \';\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    head.first->AddChildNode(std::move(child_pair.first));
    head.second = STATUS::SUCCESS;

    return head;
}

AstRet Parser::expression(AstRet &&head)
{
    reserved_token_iter_ = token_iter_;

    // auto child = std::make_unique<AssignExpr>(ASTNode::NodeKind::EXPR);
    // AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((head = simple(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

AstRet Parser::simpleDash(AstRet &&head)
{
    if (token_iter_->type != TokenType::PLUS && token_iter_->type != TokenType::MINUS) {
        std::cout << "Wrong token, expected: [+|-]" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    token_iter_++;

    reserved_token_iter_ = token_iter_;
    if ((head = simple(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        return head;
    }
    return head;
}

AstRet Parser::simple(AstRet &&head)
{
    if ((head = summand(std::move(head))).second == STATUS::SUCCESS) {
        head = simpleDash(std::move(head));
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

AstRet Parser::factorDash(AstRet &&head)
{
    if (token_iter_->type != TokenType::MUL && token_iter_->type != TokenType::DIVIDE) {
        std::cout << "Wrong token, expected: [*|/]" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    token_iter_++;

    reserved_token_iter_ = token_iter_;
    if ((head = factor(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        return head;
    }
    return head;
}

AstRet Parser::summand(AstRet &&head)
{
    if ((head = factor(std::move(head))).second == STATUS::SUCCESS) {
        head = factorDash(std::move(head));
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

AstRet Parser::factor(AstRet &&head)
{
    if ((head = primary(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    if (!term<TokenType::OPEN_BRACKET>()) {
        std::cout << "Expected \'(\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    if ((head = expression(std::move(head))).second != STATUS::SUCCESS) {
        std::cout << "Wrong Expression parsing" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    if (!term<TokenType::CLOSE_BRACKET>()) {
        std::cout << "Expected \')\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    reserved_token_iter_ = token_iter_;
    head.second = STATUS::SUCCESS;
    return head;
}

AstRet Parser::primary(AstRet &&head)
{
    std::string number = "";

    if ((head = value(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    if (term<TokenType::NUMBER>(&number)) {
        std::cout << "Found number as expression" << std::endl;

        int32_t num = atoi(number.data());

        auto child = std::make_unique<Number>(ASTNode::NodeKind::NUMBER, num);

        head.first->AddChildNode(std::move(child));

        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    token_iter_--;

    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

AstRet Parser::varDecl(AstRet &&head)
{
    std::cout << "VarDeclaration" << std::endl;

    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    if (!term<TokenType::TYPE>()) {
        std::cout << "Expected type" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<AssignExpr>(ASTNode::NodeKind::ASSIGN_EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = value(std::move(child_pair))).second != STATUS::SUCCESS) {
        std::cout << "Wrong Value parsing" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::EQUAL>()) {
        std::cout << "Expected equal sign" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        std::cout << "Wrong Expression parsing" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::SEMICOLON>()) {
        std::cout << "Expected \';\'" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    head.first->AddChildNode(std::move(child_pair.first));

    reserved_token_iter_ = token_iter_;
    return head;
}

AstRet Parser::value(AstRet &&head)
{
    std::string ident;
    if (!term<TokenType::IDENTIFIER>(&ident)) {
        std::cout << "Expected identifier" << std::endl;
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<Identifier>(ASTNode::NodeKind::IDENTIFIER, ident);

    head.first->AddChildNode(std::move(child));
    reserved_token_iter_ = token_iter_;
    head.second = STATUS::SUCCESS;
    return head;
}

bool Parser::funcParamDecl()
{
    std::cout << "FuncParamDecl" << std::endl;
    reserved_token_iter_ = token_iter_;
    return true;
}