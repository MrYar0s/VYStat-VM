#ifndef FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP
#define FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP

#include <memory>
#include <vector>

class ASTNode;

using ChildNodes = std::vector<std::unique_ptr<ASTNode>>;

class ASTNode {
public:
    enum class NodeKind {
        // Misc
        ERROR,

        // Declarations
        FUNCTION_DECL,
        PARAM_VAR_DECL,
        PROGRAM,

        // Expressions
        ASSIGN_EXPR,
        IDENTIFIER,
        NUMBER,
        STRING,
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

class VarDecl : public ASTNode {
public:
    explicit VarDecl(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~VarDecl() = default;
};

class Identifier : public ASTNode {
public:
    explicit Identifier(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~Identifier() = default;
};

class FunctionDecl : public ASTNode {
public:
    explicit FunctionDecl(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~FunctionDecl() = default;
};

class Expr : public ASTNode {
public:
    explicit Expr(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~Expr() = default;
};

class AssignExpr : public ASTNode {
public:
    explicit AssignExpr(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~AssignExpr() = default;
};

class Stmt : public ASTNode {
public:
    explicit Stmt(NodeKind kind, std::string name = "") : ASTNode(kind, name) {}
    virtual ~Stmt() = default;
};

class Number : public ASTNode {
public:
    explicit Number(NodeKind kind, int val, std::string name = "") : ASTNode(kind, name), val_(val) {}
    virtual ~Number() = default;

    int getValue() {
        return val_;
    }

private:
    int val_;
};

#endif  // FRONTEND_INCLUDE_SHRIMP_FRONTEND_ASTNODE_HPP
