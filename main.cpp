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
};


class parser {
    std::string expression;
    size_t pos;
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
                    ('a' <= expression[pos] && expression[pos] <= 'z') ||
                    ('0' <= expression[pos] && expression[pos] <= '9'))) {
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

    static string to_string(shared_ptr<node> u) {
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
            u->expression = "(!" + ul + ")";
        }
        return u->expression;
    }

public:

    shared_ptr<node> parse(string &expression) {
        this->expression = expression;
        pos = 0;
        shared_ptr<node> root = expr();
        to_string(root);
        return root;
    }
};

vector<string> string_axioms = { "a->b->a",
                          "(a->b)->(a->b->c)->(a->c)",
                          "a->b->a&b",
                          "a&b->a",
                          "a&b->b",
                          "a->a|b",
                          "b->a|b",
                          "(a->c)->(b->c)->(a|b->c)",
                          "(a->b)->(a->!b)->!a",
                          "!!a->a"};

vector<shared_ptr<node>> axioms;
vector<string> assumptions;

class checker {
    bool check_nodes_structure(shared_ptr<node> v, shared_ptr<node> u) {
        if (v->left == nullptr && (v->right == nullptr)) {
            return u->left == nullptr && u->right == nullptr;
        }
        if (v->left == nullptr && v->right != nullptr) {
            return u->left == nullptr && u->right != nullptr;
        }
        if (v->left != nullptr && v->right == nullptr) {
            return u->left != nullptr && u->right == nullptr;
        }
        if (v->left != nullptr && v->right != nullptr) {
            return u->left != nullptr && u->right != nullptr;
        }
        return false;
    }

    bool check_mapped_expr(unordered_map<char, string> &m, char u, string &v) {
        if (m.count(u)) {
            return (m[u] == v);
        }
        m[u] = v;
        return true;
    }

    bool cur_axiom(shared_ptr<node> u, shared_ptr<node> root) {
        unordered_map<char, string> exprax_to_expr;
        queue<shared_ptr<node>> qax, q;
        qax.push(u);
        q.push(root);
        while (!qax.empty()){
            shared_ptr<node> vax = qax.front();
            qax.pop();
            shared_ptr<node> v = q.front();
            q.pop();
            if (check_nodes_structure(vax, v)) {
                if (vax->op != v->op) return false;
                if (vax->op == VARIABLE) {
                    if (!check_mapped_expr(exprax_to_expr, vax->expression[0], v->expression)) return false;
                }
                if (vax->left != nullptr) {
                    qax.push(vax->left);
                    q.push(v->left);
                }
                if (vax->right != nullptr) {
                    qax.push(vax->right);
                    q.push(v->right);
                }
            } else {
                if (vax->left == nullptr && vax->right == nullptr) {
                    if (!check_mapped_expr(exprax_to_expr, vax->expression[0], v->expression)) return false;
                } else return false;
            }
        }
        return true;
    }

public:
    bool check_axioms(shared_ptr<node> root) {
        for (shared_ptr<node> u: axioms) {
            if (cur_axiom(u, root)) {
                return true;
            }
        }
        return false;
    }

    bool check_assumtions(shared_ptr<node> root) {

    }

};


int main() {
    freopen("in.txt", "r", stdin);
    freopen("out.txt", "w", stdout);
    string s;
    getline(cin, s);

    parser p;



    return 0;
}