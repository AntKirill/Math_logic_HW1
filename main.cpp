#include <bits/stdc++.h>

using namespace std;

enum operation_type {
    BINARY, UNARY
};
enum token {
    IMPL, OR, AND, NOT, BRACKET, VARIABLE, END
};

struct node {

    operation_type type;
    token op;
    string expression;

    shared_ptr<node> left, right;

    node() : left(nullptr), right(nullptr) { }

    string to_string(shared_ptr<node> u) {
        string ul(""), ur("");
        if (u->left != nullptr) {
            ul = to_string(u->left);
        }
        if (u->right != nullptr) {
            ur = to_string(u->right);
        }
        string sign;
        switch (u->op) {
            case IMPL:
                sign = "->";
                break;
            case OR:
                sign = "|";
                break;
            case AND:
                sign = "&";
                break;
            case NOT:
                sign = "!";
                break;
            default:
                break;
        }
        if (ur != "") {
            u->expression = "(" + ul + sign + ur + ")";
        } else if (ul != "") {
            u->expression = "(!" + ul + ')';
        }
        return u->expression;
    }
};


class tree {

    const std::string expression;
    size_t pos = 0;
    token cur_token;
    operation_type cur_operation_type;
    string cur_variable;

    void next_token() {
        while (pos < expression.length()) {
            switch (expression[pos]) {
                case ' ':
                    pos++;
                    continue;
                case '&' :
                    cur_operation_type = BINARY;
                    cur_token = AND;
                    break;
                case '|':
                    cur_operation_type = BINARY;
                    cur_token = OR;
                    break;
                case '!':
                    cur_operation_type = UNARY;
                    cur_token = NOT;
                    break;
                case '(':
                    cur_token = BRACKET;
                    break;
                case ')':
                    cur_token = BRACKET;
                    break;
                default:
                    break;
            }
            if (pos < expression.length() - 1) {
                if (expression[pos] == '-' && expression[pos + 1] == '>') {
                    cur_token = IMPL;
                    cur_operation_type = BINARY;
                    pos += 2;
                    return;
                }
            }
            string tmp;
            while ((pos < expression.length()) &&
                   (('A' <= expression[pos] && expression[pos] <= 'Z') ||
                    ('a' <= expression[pos] && expression[pos] <= 'z')
                    || ('0' <= expression[pos] && expression[pos] <= '9'))) {
                cur_token = VARIABLE;
                tmp += expression[pos];
                pos++;
            }
            if (cur_token != VARIABLE) pos++;
            else cur_variable = tmp;
            return;
        }
        cur_token = END;
    }

    shared_ptr<node> expr() {
        next_token();
        shared_ptr<node> sub_root = disj();
        if (cur_token == IMPL) {
            shared_ptr<node> new_sub_root(new node());
            new_sub_root->left = sub_root;
            new_sub_root->right = expr();
            new_sub_root->op = IMPL;
            sub_root = new_sub_root;
        }
        while (cur_token == IMPL) {
            shared_ptr<node> right_children(new node());
            right_children->op = IMPL;
            right_children->left = sub_root->right;
            sub_root->right = right_children;
            right_children->right = expr();
        }
        return sub_root;
    }

    shared_ptr<node> disj() {
        shared_ptr<node> sub_root = conj();
        while (cur_token == OR) {
            next_token();
            shared_ptr<node> new_sub_root(new node());
            new_sub_root->op = OR;
            new_sub_root->left = sub_root;
            new_sub_root->right = conj();
            sub_root = new_sub_root;
        }
        return sub_root;
    }

    shared_ptr<node> conj() {
        shared_ptr<node> sub_root = negetion();
        while (cur_token == AND) {
            next_token();
            shared_ptr<node> new_sub_root(new node());
            new_sub_root->op = AND;
            new_sub_root->left = sub_root;
            new_sub_root->right = negetion();
            sub_root = new_sub_root;
        }
        return sub_root;
    }

    shared_ptr<node> negetion() {
        if (cur_token == VARIABLE) {
            shared_ptr<node> sub(new node());
            sub->op = VARIABLE;
            sub->expression = cur_variable;
            next_token();
            return sub;
        } else if (cur_token == NOT) {
            next_token();
            shared_ptr<node> sub(new node());
            sub->op = NOT;
            sub->left = negetion();
            return sub;
        } else if (cur_token == BRACKET) {
            shared_ptr<node> sub = expr();
            next_token();
            return sub;
        }
        return nullptr;
    }

public:

    tree(const std::string &expression) : expression(expression) { }

    shared_ptr<node> parse() {
        return expr();
    }

};

int main() {
    freopen("in.txt", "r", stdin);
    freopen("out.txt", "w", stdout);
    string s;
    getline(cin, s);

    tree p(s);

    shared_ptr<node> root = p.parse();
    string st = root->to_string(root);

    cout << st;
    return 0;
}