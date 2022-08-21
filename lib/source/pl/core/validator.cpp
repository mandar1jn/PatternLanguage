#include <pl/core/validator.hpp>

#include <pl/core/ast/ast_node.hpp>
#include <pl/core/ast/ast_node_variable_decl.hpp>
#include <pl/core/ast/ast_node_type_decl.hpp>
#include <pl/core/ast/ast_node_struct.hpp>
#include <pl/core/ast/ast_node_union.hpp>
#include <pl/core/ast/ast_node_enum.hpp>

#include <unordered_set>
#include <string>

namespace pl::core {

    bool Validator::validate(const std::string &sourceCode, const std::vector<std::shared_ptr<ast::ASTNode>> &ast) {
        std::unordered_set<std::string> identifiers;
        std::unordered_set<std::string> types;

        this->m_recursionDepth++;
        try {

            for (const auto &node : ast) {
                if (node == nullptr)
                    err::V0001.throwError("Null-Pointer found in AST.", "This is a parser bug. Please report it on GitHub.");

                this->m_lastNode = node.get();

                if (this->m_recursionDepth > this->m_maxRecursionDepth)
                    err::V0003.throwError(fmt::format("Type recursion depth exceeded set limit of '{}'.", this->m_maxRecursionDepth), "If this is intended, try increasing the limit using '#pragma eval_depth <new_limit>'.");

                if (auto variableDeclNode = dynamic_cast<ast::ASTNodeVariableDecl *>(node.get()); variableDeclNode != nullptr) {
                    if (!identifiers.insert(variableDeclNode->getName().data()).second)
                        err::V0002.throwError(fmt::format("Redefinition of variable '{0}", variableDeclNode->getName()));

                    if (!this->validate(sourceCode, hlp::moveToVector<std::shared_ptr<ast::ASTNode>>(variableDeclNode->getType()->clone())))
                        return false;
                } else if (auto typeDeclNode = dynamic_cast<ast::ASTNodeTypeDecl *>(node.get()); typeDeclNode != nullptr) {
                    if (!types.insert(typeDeclNode->getName().c_str()).second)
                        err::V0002.throwError(fmt::format("Redefinition of type '{0}", typeDeclNode->getName()));

                    if (!typeDeclNode->isForwardDeclared())
                        if (!this->validate(sourceCode, hlp::moveToVector<std::shared_ptr<ast::ASTNode>>(typeDeclNode->getType()->clone())))
                            return false;
                } else if (auto structNode = dynamic_cast<ast::ASTNodeStruct *>(node.get()); structNode != nullptr) {
                    if (!this->validate(sourceCode, structNode->getMembers()))
                        return false;
                } else if (auto unionNode = dynamic_cast<ast::ASTNodeUnion *>(node.get()); unionNode != nullptr) {
                    if (!this->validate(sourceCode, unionNode->getMembers()))
                        return false;
                } else if (auto enumNode = dynamic_cast<ast::ASTNodeEnum *>(node.get()); enumNode != nullptr) {
                    std::unordered_set<std::string> enumIdentifiers;
                    for (auto &[name, value] : enumNode->getEntries()) {
                        if (!enumIdentifiers.insert(name).second)
                            err::V0002.throwError(fmt::format("Redefinition of enum entry '{0}", name));
                    }
                }
            }

        } catch (err::ValidatorError::Exception &e) {
            if (this->m_lastNode != nullptr) {
                auto line = this->m_lastNode->getLine();
                auto column = this->m_lastNode->getColumn();

                this->m_error = err::PatternLanguageError(e.format(sourceCode, line, column), line, column);
            }
            else
                this->m_error = err::PatternLanguageError(e.format(sourceCode, 1, 1), 1, 1);

            this->m_recursionDepth = 0;

            return false;
        }

        this->m_recursionDepth--;

        return true;
    }
}