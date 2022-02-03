#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

class ValueExpression : public Expression {
public:
    ValueExpression(int value) : value_(value) {}

    int Evaluate() const override {
        return value_;
    }

    string ToString() const override {
        return to_string(value_);
    }

private:
    int value_;
};

class OpExpression : public Expression {
public:
    OpExpression(char op, ExpressionPtr lhs, ExpressionPtr rhs)
        : op_(op), lhs_(move(lhs)), rhs_(move(rhs)) {}

    int Evaluate() const override = 0;

    string ToString() const final {
        return "(" + lhs_->ToString() + ")" + op_ + "(" + rhs_->ToString() + ")";
    }

protected:
    char op_;
    ExpressionPtr lhs_;
    ExpressionPtr rhs_;
};

class SumExpression : public OpExpression {
public:
    SumExpression(ExpressionPtr lhs, ExpressionPtr rhs) : OpExpression('+', move(lhs), move(rhs)) {}

    int Evaluate() const override {
        return lhs_->Evaluate() + rhs_->Evaluate();
    }
};

class ProductExpression : public OpExpression {
public:
    ProductExpression(ExpressionPtr lhs, ExpressionPtr rhs) : OpExpression('*', move(lhs), move(rhs)) {}

    int Evaluate() const override {
        return lhs_->Evaluate() * rhs_->Evaluate();
    }
};

ExpressionPtr Value(int value) {
    return make_unique<ValueExpression>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<SumExpression>(move(left), move(right));
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<ProductExpression>(move(left), move(right));
}

string Print(const Expression* e) {
    if (!e) {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

void Test() {
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}