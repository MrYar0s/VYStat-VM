#ifndef FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP
#define FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include "shrimp/common/types.hpp"

class ASTNode;

using ChildNodes = std::vector<std::unique_ptr<ASTNode>>;

enum class ValueType { INT, FLOAT, STRING };

class ASTNode {
public:
    enum class NodeKind {
        // Misc
        ERROR,

        // Declarations
        FUNCTION_DECL,
        PARAM_VAR_DECL,
        PROGRAM,

        // Statements
        RETURN_STATEMENT,
        IF_STATEMENT,
        ARRAY,

        IF_BODY,

        // General Exprs
        EXPR,

        FUNCTION_CALL,

        // Expressions
        ASSIGN_EXPR,
        IDENTIFIER,
        NUMBER,
        STRING,
    };

    enum class IntrinsicType {
        NONE,
        SCAN,
        PRINT,
        SQRT,
        SIN,
        COS,
        CONCAT,
        SUBSTR,
    };

public:
    explicit ASTNode(NodeKind kind, std::string name = "") : kind_ {kind}, name_ {name} {}
    virtual ~ASTNode() = default;

public:
    NodeKind GetKind() const
    {
        return kind_;
    }
    const std::string &GetName() const
    {
        return name_;
    }
    const ChildNodes &GetChildrenNodes() const
    {
        return childs_;
    }

    void setName(const std::string &str)
    {
        name_ = str;
    }

    void AddChildNode(std::unique_ptr<ASTNode> &&node)
    {
        childs_.emplace_back(std::move(node));
    }

    void AddChildNodes(std::vector<std::unique_ptr<ASTNode>> &&nodes)
    {
        childs_.insert(childs_.end(), std::make_move_iterator(nodes.begin()), std::make_move_iterator(nodes.end()));
    }

private:
    NodeKind kind_;
    std::string name_;
    ChildNodes childs_;
};

class RetStmt : public ASTNode {
public:
    explicit RetStmt(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~RetStmt() = default;
};

class Identifier : public ASTNode {
public:
    explicit Identifier(NodeKind kind, ValueType type, std::string name = "") : ASTNode(kind, name), type_(type) {}
    virtual ~Identifier() = default;

    auto getType() const
    {
        return type_;
    }

private:
    ValueType type_;
};

class Array : public ASTNode {
public:
    explicit Array(NodeKind kind, ValueType type, std::string name = "") : ASTNode(kind, name), type_(type) {}
    virtual ~Array() = default;

    auto getType() const
    {
        return type_;
    }

private:
    ValueType type_;
};

class FunctionDecl : public ASTNode {
public:
    explicit FunctionDecl(NodeKind kind, std::vector<std::pair<std::string, ValueType>> args, std::string name = "")
        : ASTNode(kind, name), args_(args)
    {
    }
    virtual ~FunctionDecl() = default;

    const auto &getArgs() const
    {
        return args_;
    }

private:
    std::vector<std::pair<std::string, ValueType>> args_;
};

class AssignExpr : public ASTNode {
public:
    explicit AssignExpr(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~AssignExpr() = default;
};

class Expr : public ASTNode {
public:
    explicit Expr(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~Expr() = default;
};

class IfStmt : public ASTNode {
public:
    explicit IfStmt(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~IfStmt() = default;
};

class IfBody : public ASTNode {
public:
    explicit IfBody(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~IfBody() = default;
};

class FunctionCall : public ASTNode {
public:
    explicit FunctionCall(NodeKind kind, std::vector<std::string> args, std::string name = "",
                          IntrinsicType type = IntrinsicType::NONE)
        : ASTNode(kind, name), args_(args), type_(type)
    {
    }
    virtual ~FunctionCall() = default;

    const auto &getArgs() const
    {
        return args_;
    }

    auto getIntrinsicType() const
    {
        return type_;
    }

private:
    std::vector<std::string> args_;
    IntrinsicType type_;
};

class Number : public ASTNode {
public:
    union value_t {
        int int_val_;
        float float_val_;
    };

    explicit Number(NodeKind kind, uint64_t val, ValueType type, shrimp::R8Id tmp_reg_num, std::string name = "")
        : ASTNode(kind, name), val_(val), type_(type)
    {
        tmp_reg_name_ = "<tmp>" + std::to_string(tmp_reg_num);
    }
    virtual ~Number() = default;

    uint64_t getValue()
    {
        return val_;
    }

    const std::string &getTmpName() const
    {
        return tmp_reg_name_;
    }

    auto getType() const
    {
        return type_;
    }

private:
    uint64_t val_;
    std::string tmp_reg_name_;
    ValueType type_;
};

class String : public ASTNode {
public:
    union value_t {
        int int_val_;
        float float_val_;
    };

    explicit String(NodeKind kind, std::string str, ValueType type, shrimp::R8Id tmp_reg_num, std::string name = "")
        : ASTNode(kind, name), str_(str), type_(type)
    {
        tmp_reg_name_ = "<str>" + std::to_string(tmp_reg_num);
    }
    virtual ~String() = default;

    auto getValue()
    {
        return str_;
    }

    const std::string &getTmpName() const
    {
        return tmp_reg_name_;
    }

    auto getType() const
    {
        return type_;
    }

private:
    std::string str_;
    std::string tmp_reg_name_;
    ValueType type_;
};

#endif  // FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP
