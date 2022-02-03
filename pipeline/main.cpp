#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
    string from;
    string to;
    string body;
};

istream& operator>>(istream& is, Email& email) {
    getline(is, email.from);
    getline(is, email.to);
    getline(is, email.body);

    return is;
}

class Worker {
public:
    virtual ~Worker() = default;

    virtual void Process(unique_ptr<Email> email) {}

    virtual void Run() {
        throw logic_error("Unimplemented");
    }

protected:
    void PassOn(unique_ptr<Email> email) const {
        if (next_worker)
            next_worker->Process(move(email));
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        next_worker = move(next);
    }

private:
    unique_ptr<Worker> next_worker;
};


class Reader : public Worker {
public:
    Reader(istream& input) : input_stream(input) {}

    void Run() override {
        Email email;

        while (input_stream >> email)
            PassOn(make_unique<Email>(email));
    }

private:
    istream& input_stream;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email&)>;

    Filter(Function func) : func_(move(func)) {}

    void Process(unique_ptr<Email> email) override {
        if (func_(*email))
            PassOn(move(email));
    }

private:
    Function func_;
};


class Copier : public Worker {
public:
    Copier(string to) : recipient(move(to)) {}

    void Process(unique_ptr<Email> email) override {
        Email email_copy = *email;

        PassOn(move(email));

        if (email_copy.to != recipient) {
            email_copy.to = recipient;
            PassOn(make_unique<Email>(email_copy));
        }
    }

private:
    string recipient;
};


class Sender : public Worker {
public:
    Sender(ostream& os) : output(os) {}

    void Process(unique_ptr<Email> email) override {
        output << email->from << "\n" << email->to << "\n" << email->body << "\n";
        PassOn(move(email));
    }

private:
    ostream& output;
};


class PipelineBuilder {
public:
    explicit PipelineBuilder(istream& in) {
        first_worker = make_unique<Reader>(in);
        last_worker = first_worker.get();
    }

    PipelineBuilder& FilterBy(Filter::Function filter) {
        ConnectWorker(make_unique<Filter>(filter));
        return *this;
    }

    PipelineBuilder& CopyTo(string recipient) {
        ConnectWorker(make_unique<Copier>(recipient));
        return *this;
    }

    PipelineBuilder& Send(ostream& out) {
        ConnectWorker(make_unique<Sender>(out));
        return *this;
    }

    unique_ptr<Worker> Build() {
        return move(first_worker);
    }

private:
    unique_ptr<Worker> first_worker;
    Worker* last_worker;

    void ConnectWorker(unique_ptr<Worker> worker) {
        Worker* new_worker = worker.get();

        last_worker->SetNext(move(worker));
        last_worker = new_worker;
    }
};


void TestSanity() {
    string input = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "ralph@example.com\n"
            "erich@example.com\n"
            "I do not make mistakes of that kind\n"
    );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email& email) {
        return email.from == "erich@example.com";
    });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "erich@example.com\n"
            "richard@example.com\n"
            "Are you sure you pressed the right button?\n"
    );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}