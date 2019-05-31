/**
 * Inspired by Analyzing Knuth machine
 * Simple implementation for grammar:
 B ::= R | '(' E ')'
 R ::= E '=' E
 E ::= 'a' | 'b' | '(' E '+' E ')'
 */

#include <iostream>
#include <functional>
#include <vector>
#include <string>

#include "perv.h"

using namespace std;

enum option_to_do {
    N = 0, // continue matching
    T, // stop matching with code TRUE
    F, // stop matching with code FALSE
    OK,
    ERROR = static_cast<size_t>(-1), // stop matching with code ERROR
};

ostream& operator<<(ostream& outstream, option_to_do instance) {
    static string representation[] = { "NEXT", "TRUE", "FALSE", "OK", "ERROR" };
    return outstream << representation[instance];
}

template <typename Token>
struct operation_or_token {
	union {
		Token token;
		size_t operation_key;
	};
	bool is_token;
	operation_or_token(Token t) : token(t), is_token(true) {}
	operation_or_token(int o_k) //L.i.A.
        : operation_key(size_t(o_k)), is_token(false) {}
};

template <typename OperationOrToken>
struct operation_branch {
	OperationOrToken operation_or_token; option_to_do when_true; option_to_do when_false;
};


enum operation_key {
	S, B, R, E, END
};

template <typename Token, typename TokensIterator, typename Tree,
    typename OperationBranchesType = vector<operation_branch<operation_or_token<Token> > > >
class knuth_machine {
public:
    //typedef vector<operation_type> operations_type;
    typedef vector<OperationBranchesType> table_type;
    knuth_machine(table_type table, TokensIterator begin, TokensIterator end)
        : table(table), begin(begin), end(end) {}

    struct returned {
        option_to_do flag; Tree* tree; TokensIterator h;
        void clear() {
            tree->clear();
            delete tree;
        }
    };

    returned start(size_t entry = 0) {
        return handle(entry, begin);
    }

    returned handle(size_t operation_key, TokensIterator h) {
        OperationBranchesType row = table[operation_key];
        Tree* result_tree = new Tree(operation_key);
        option_to_do current_state = N;
        TokensIterator current_h = h;
        for (const auto& operation_branch : row) {
            if (operation_branch.operation_or_token.is_token) {
                if (current_h == end) {
                    current_state = F;
                } else {
                    if (*current_h == operation_branch.operation_or_token.token) {
                        ++current_h;
                        current_state = operation_branch.when_true;
                    } else {
                        current_state = operation_branch.when_false;
                    }
                }
            } else {
                if (operation_branch.operation_or_token.operation_key == END) {
                    if (current_h == end) {
                        current_state = operation_branch.when_true;
                    }
                    else {
                        current_state = operation_branch.when_false;
                    }
                } else {
                    auto inner = handle(operation_branch.operation_or_token.operation_key, current_h);
                    switch (inner.flag) {
                    case T:
                        current_state = operation_branch.when_true;
                        result_tree->add_branch(inner.tree);
                        current_h = inner.h;
                        break;
                    case F:
                        current_state = operation_branch.when_false;
                        break;
                    default:
                        current_state = ERROR;
                    }
                }
            }
            switch (current_state) {
            case N: break;
            case T: return { T, result_tree, current_h };
            case F: return { F, nullptr, h };
            case ERROR: return { ERROR, nullptr, h };
            case OK: return { OK, result_tree, current_h };
            }
        }
        return { ERROR, result_tree, current_h };
    }

private:
    table_type table;
    TokensIterator begin;
    TokensIterator end;
};

typedef char token; // of course NOT char

template <typename Value>
struct node {
	Value value;
	vector<node*> branches;
	node(size_t operation_key) : value(to_value(operation_key)) {}
	void add_branch(node* branch) {
		branches.push_back(branch);
	}
    void clear() {
        for (auto branch : branches) {
            branch->clear();
            delete branch;
        }
    }
private:
    static Value to_value(size_t operation_key) {
        return to_string(operation_key);
    }
	template <typename V>
	friend ostream& operator<<(ostream&, const node<V>&);
};

template <typename V>
void print_node(ostream& stream, const node<V>& instance, int level = 0) {
	for (int i = 0; i < 2 * level; ++i) {
		stream << '.';
	}
	stream << instance.value << endl;
	for (const auto& branch : instance.branches) {
		print_node(stream, *branch, 1 + level);
	}
}

template <typename V>
ostream& operator<<(ostream& stream, const node<V>& instance) {
	print_node(stream, instance);
	return stream;
}

typedef node<string> tree;
typedef knuth_machine<token, vector<token>::iterator, tree> analyzer;

int main() {
	string in = "a=(b+a)";
	vector<token> tokens(in.begin(), in.end());
	analyzer::table_type table =
	{
		{
			{ B , N, ERROR},
			{END, OK, ERROR}
		},
		{
			{ R , T, N},
			{'(', N, F},
			{ B , N, F},
			{')', T, F},
		},
		{
			{ E , N, F},
			{'=', N, F},
			{ E , T, F}
		},
		{
			{'a', T, N},
			{'b', T, N},
			{'(', N, F},
			{ E , N, F},
			{'+', N, F},
			{ E , N, F},
			{')', T, F}
		}
	};
	analyzer knuth_analyzer(table, tokens.begin(), tokens.end());
    cleaner<analyzer::returned> result(knuth_analyzer.start());
    cout << result.flag << endl;
	cout << *result.tree << endl;
    cout << hex << static_cast<size_t>(-1) << endl;
	return 0;
}
/**
 * WON'T WORK FOR GRAMMAR WITH RULE(S):
 * S ::= E + E
 *     | E - E
 * because of adding extra branches from false calls
 * or no
 * yes
 **/
