#include <cassert>
#include <memory>
#include <shrimp/parser.hpp>
#include <shrimp/frontend/astnode.hpp>
#include <iostream>
#include <shrimp/lexer.hpp>
#include <utility>
#include <shrimp/common/bitops.hpp>

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
    return funcDecl();
}

STATUS Parser::programDecl2()
{
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
    if (term<TokenType::END>()) {
        token_iter_--;
        return STATUS::END;
    }
    token_iter_--;

    std::string ident = "";

    if (!term<TokenType::FUNCTION>()) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::IDENTIFIER>(&ident)) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::OPEN_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    auto funcArgs = funcParamDecl();

    if (!term<TokenType::CLOSE_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::OPEN_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    auto head =
        AstRet(std::make_unique<FunctionDecl>(ASTNode::NodeKind::FUNCTION_DECL, funcArgs, ident), STATUS::SUCCESS);

    reserved_token_iter_ = token_iter_;
    if ((head = stmtsDecl(std::move(head))).second == STATUS::FAIL) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    if (!term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        return STATUS::FAIL;
    }

    root_.first->AddChildNode(std::move(head.first));

    reserved_token_iter_ = token_iter_;
    return STATUS::SUCCESS;
}

AstRet Parser::stmtsDecl(AstRet &&head)
{
    if ((head = assignmentExpr(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = varDecl(std::move(head), true)).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = varDecl(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = retStmt(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = ifStmt(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = forStmt(std::move(head))).second == STATUS::SUCCESS) {
        head = stmtsDeclDash(std::move(head));
        return head;
    }
    if ((head = functionCall(std::move(head))).second == STATUS::SUCCESS) {
        if (!term<TokenType::SEMICOLON>()) {
            token_iter_ = reserved_token_iter_;
            head.second = STATUS::FAIL;
            return head;
        }
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

AstRet Parser::assignmentExpr(AstRet &&head)
{
    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    auto child = std::make_unique<AssignExpr>(ASTNode::NodeKind::ASSIGN_EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = value(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::EQUAL>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::SEMICOLON>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    assert(child_pair.first != nullptr);
    head.first->AddChildNode(std::move(child_pair.first));

    head.second = STATUS::SUCCESS;
    reserved_token_iter_ = token_iter_;
    return head;
}

AstRet Parser::forStmt(AstRet &&head)
{
    if (!term<TokenType::FOR>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::OPEN_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<ForStmt>(ASTNode::NodeKind::FOR_STATEMENT);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = varDecl(std::move(child_pair), false)).second != STATUS::SUCCESS &&
        (child_pair = assignmentExpr(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::SEMICOLON>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if ((child_pair = assignmentExpr(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::CLOSE_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::OPEN_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto stmts = AstRet(std::make_unique<ForBody>(ASTNode::NodeKind::FOR_BODY), STATUS::SUCCESS);

    reserved_token_iter_ = token_iter_;
    if ((stmts = stmtsDecl(std::move(stmts))).second == STATUS::FAIL) {
        token_iter_ = reserved_token_iter_;
        return head;
    }
    child_pair.first->AddChildNode(std::move(stmts.first));

    if (!term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    head.first->AddChildNode(std::move(child_pair.first));

    head.second = STATUS::SUCCESS;
    reserved_token_iter_ = token_iter_;
    return head;
}

AstRet Parser::ifStmt(AstRet &&head)
{
    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    if (!term<TokenType::IF>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::OPEN_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<IfStmt>(ASTNode::NodeKind::IF_STATEMENT);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::CLOSE_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::OPEN_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto stmts = AstRet(std::make_unique<IfBody>(ASTNode::NodeKind::IF_BODY), STATUS::SUCCESS);

    reserved_token_iter_ = token_iter_;
    if ((stmts = stmtsDecl(std::move(stmts))).second == STATUS::FAIL) {
        token_iter_ = reserved_token_iter_;
        return head;
    }

    child_pair.first->AddChildNode(std::move(stmts.first));

    if (!term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    head.first->AddChildNode(std::move(child_pair.first));

    head.second = STATUS::SUCCESS;
    reserved_token_iter_ = token_iter_;
    return head;
}

AstRet Parser::retStmt(AstRet &&head)
{
    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    if (!term<TokenType::RETURN>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<RetStmt>(ASTNode::NodeKind::RETURN_STATEMENT);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!term<TokenType::SEMICOLON>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    head.first->AddChildNode(std::move(child_pair.first));
    head.second = STATUS::SUCCESS;

    return head;
}

AstRet Parser::expressionDash(AstRet &&head)
{
    if (token_iter_->type != TokenType::IS_LESS && token_iter_->type != TokenType::IS_EQUAL &&
        token_iter_->type != TokenType::IS_GREATER) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    head.first->setName(token_iter_->value);
    token_iter_++;

    reserved_token_iter_ = token_iter_;
    if ((head = expression(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        return head;
    }
    return head;
}

AstRet Parser::expression(AstRet &&head)
{
    reserved_token_iter_ = token_iter_;

    auto child = std::make_unique<Expr>(ASTNode::NodeKind::EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);
    if ((child_pair = simple(std::move(child_pair))).second == STATUS::SUCCESS) {
        child_pair = expressionDash(std::move(child_pair));
        reserved_token_iter_ = token_iter_;
        head.first->AddChildNode(std::move(child_pair.first));
        head.second = STATUS::SUCCESS;
        return head;
    }

    assert(child_pair.first != nullptr);
    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

AstRet Parser::simpleDash(AstRet &&head)
{
    if (token_iter_->type != TokenType::PLUS && token_iter_->type != TokenType::MINUS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    head.first->setName(token_iter_->value);
    token_iter_++;

    auto child = std::make_unique<Expr>(ASTNode::NodeKind::EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    reserved_token_iter_ = token_iter_;
    if ((child_pair = simple(std::move(child_pair))).second == STATUS::SUCCESS) {
        head.first->AddChildNode(std::move(child_pair.first));
        reserved_token_iter_ = token_iter_;
        return head;
    }
    return head;
}

AstRet Parser::simple(AstRet &&head)
{
    assert(head.first != nullptr);
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
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    head.first->setName(token_iter_->value);
    token_iter_++;

    auto child = std::make_unique<Expr>(ASTNode::NodeKind::EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    reserved_token_iter_ = token_iter_;
    if ((child_pair = factor(std::move(child_pair))).second == STATUS::SUCCESS) {
        head.first->AddChildNode(std::move(child_pair.first));
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
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    if ((head = expression(std::move(head))).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    if (!term<TokenType::CLOSE_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }
    reserved_token_iter_ = token_iter_;
    head.second = STATUS::SUCCESS;
    return head;
}

ASTNode::IntrinsicType Parser::parseIntrinsicType(std::string &name)
{
    if (term<TokenType::SCAN>(&name)) {
        return ASTNode::IntrinsicType::SCAN;
    }
    token_iter_--;
    if (term<TokenType::PRINT>(&name)) {
        return ASTNode::IntrinsicType::PRINT;
    }
    token_iter_--;
    if (term<TokenType::SQRT>(&name)) {
        return ASTNode::IntrinsicType::SQRT;
    }
    token_iter_--;
    if (term<TokenType::CONCAT>(&name)) {
        return ASTNode::IntrinsicType::CONCAT;
    }
    token_iter_--;
    if (term<TokenType::SUBSTR>(&name)) {
        return ASTNode::IntrinsicType::SUBSTR;
    }
    token_iter_--;
    return ASTNode::IntrinsicType::NONE;
}

AstRet Parser::functionCall(AstRet &&head)
{
    std::string name;

    ASTNode::IntrinsicType type = ASTNode::IntrinsicType::NONE;

    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    if (!term<TokenType::IDENTIFIER>(&name)) {
        token_iter_--;
        type = parseIntrinsicType(name);
        if (type == ASTNode::IntrinsicType::NONE) {
            head.second = STATUS::FAIL;
            return head;
        }
    }

    if (!term<TokenType::OPEN_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    const auto &args = funcParamCall();

    if (!term<TokenType::CLOSE_BRACKET>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<FunctionCall>(ASTNode::NodeKind::FUNCTION_CALL, args, name, type);

    head.first->AddChildNode(std::move(child));

    reserved_token_iter_ = token_iter_;
    head.second = STATUS::SUCCESS;
    return head;
}

AstRet Parser::primary(AstRet &&head)
{
    if ((head = functionCall(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    std::string variable = "";
    std::string neg = "";

    // Parsing of negative value
    if (term<TokenType::MINUS>()) {
        neg = "-";
    } else {
        token_iter_--;
    }

    if (term<TokenType::NUMBER>(&variable)) {
        auto val_type = variable.find('.');
        if (val_type == std::string::npos) {
            int32_t num = atoi((neg + variable).data());
            uint64_t val = shrimp::bit::castToWritable<int>(num);
            auto child = std::make_unique<Number>(ASTNode::NodeKind::NUMBER, val, ValueType::INT, num_of_tmp_regs_);
            num_of_tmp_regs_++;
            head.first->AddChildNode(std::move(child));
        } else {
            float num = atof((neg + variable).data());
            uint64_t val = shrimp::bit::castToWritable<float>(num);
            auto child = std::make_unique<Number>(ASTNode::NodeKind::NUMBER, val, ValueType::FLOAT, num_of_tmp_regs_);
            num_of_tmp_regs_++;
            head.first->AddChildNode(std::move(child));
        }
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    token_iter_--;

    if (term<TokenType::STRING>(&variable)) {
        std::string str = variable;
        auto child = std::make_unique<String>(ASTNode::NodeKind::STRING, str, ValueType::STRING, num_of_tmp_regs_);
        num_of_tmp_regs_++;
        head.first->AddChildNode(std::move(child));
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }
    token_iter_--;

    if ((head = value(std::move(head))).second == STATUS::SUCCESS) {
        reserved_token_iter_ = token_iter_;
        head.second = STATUS::SUCCESS;
        return head;
    }

    token_iter_ = reserved_token_iter_;
    head.second = STATUS::FAIL;
    return head;
}

static ValueType castToVarType(std::string type)
{
    ValueType var_type = ValueType::INT;
    if (type == "float") {
        var_type = ValueType::FLOAT;
    } else if (type == "string") {
        var_type = ValueType::STRING;
    }
    return var_type;
}

AstRet Parser::varDecl(AstRet &&head, bool is_array)
{
    if (term<TokenType::CLOSE_FIG_BRACKET>()) {
        token_iter_--;
        head.second = STATUS::END;
        return head;
    }
    token_iter_--;

    std::string type;

    if (!term<TokenType::TYPE>(&type)) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    auto child = std::make_unique<AssignExpr>(ASTNode::NodeKind::ASSIGN_EXPR);
    AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);

    auto var_type = castToVarType(type);

    if ((child_pair = value(std::move(child_pair), var_type, true)).second != STATUS::SUCCESS) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (!is_array) {
        if (!term<TokenType::EQUAL>()) {
            token_iter_ = reserved_token_iter_;
            head.second = STATUS::FAIL;
            return head;
        }

        if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
            token_iter_ = reserved_token_iter_;
            head.second = STATUS::FAIL;
            return head;
        }
    }

    if (!term<TokenType::SEMICOLON>()) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    assert(child_pair.first != nullptr);
    head.first->AddChildNode(std::move(child_pair.first));

    head.second = STATUS::SUCCESS;
    reserved_token_iter_ = token_iter_;
    return head;
}

AstRet Parser::value(AstRet &&head, ValueType type, bool need_to_init)
{
    std::string ident;
    if (!term<TokenType::IDENTIFIER>(&ident)) {
        token_iter_ = reserved_token_iter_;
        head.second = STATUS::FAIL;
        return head;
    }

    if (need_to_init) {
        ident_to_type.insert({ident, type});
    }
    type = ident_to_type[ident];

    if (term<TokenType::OPEN_SQUARE_BRACKET>()) {
        auto child = std::make_unique<Array>(ASTNode::NodeKind::ARRAY, type, ident);
        AstRet child_pair = std::make_pair(std::move(child), STATUS::SUCCESS);
        if (need_to_init) {
            if ((child_pair = primary(std::move(child_pair))).second != STATUS::SUCCESS) {
                token_iter_ = reserved_token_iter_;
                head.second = STATUS::FAIL;
            }
        } else {
            if ((child_pair = expression(std::move(child_pair))).second != STATUS::SUCCESS) {
                token_iter_ = reserved_token_iter_;
                head.second = STATUS::FAIL;
            }
        }
        if (!term<TokenType::CLOSE_SQUARE_BRACKET>()) {
            token_iter_ = reserved_token_iter_;
            head.second = STATUS::FAIL;
        }
        head.first->AddChildNode(std::move(child_pair.first));
        head.second = STATUS::SUCCESS;
        return head;
    } else {
        token_iter_--;
    }

    auto child = std::make_unique<Identifier>(ASTNode::NodeKind::IDENTIFIER, type, ident);

    head.first->AddChildNode(std::move(child));
    head.second = STATUS::SUCCESS;
    return head;
}

std::vector<std::string> Parser::funcParamCall()
{
    std::vector<std::string> funcArgs;

    for (int i = 0; i < 4; i++) {
        std::string name = "";
        if (!term<TokenType::IDENTIFIER>(&name)) {
            token_iter_--;
            reserved_token_iter_ = token_iter_;
            return funcArgs;
        }

        funcArgs.emplace_back(name);
        if (!term<TokenType::COMMA>()) {
            token_iter_--;
            reserved_token_iter_ = token_iter_;
            return funcArgs;
        }
    }

    reserved_token_iter_ = token_iter_;
    return funcArgs;
}

std::vector<std::pair<std::string, ValueType>> Parser::funcParamDecl()
{
    std::vector<std::pair<std::string, ValueType>> funcArgs;

    for (int i = 0; i < 4; i++) {
        std::string type = "";
        if (!term<TokenType::TYPE>(&type)) {
            token_iter_--;
            reserved_token_iter_ = token_iter_;
            return funcArgs;
        }
        std::string name = "";
        if (!term<TokenType::IDENTIFIER>(&name)) {
            token_iter_--;
            reserved_token_iter_ = token_iter_;
            return funcArgs;
        }

        funcArgs.emplace_back(std::make_pair(name, castToVarType(type)));
        if (!term<TokenType::COMMA>()) {
            token_iter_--;
            reserved_token_iter_ = token_iter_;
            return funcArgs;
        }
    }

    reserved_token_iter_ = token_iter_;
    return funcArgs;
}