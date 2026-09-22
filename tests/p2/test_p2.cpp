// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

class TestInput : public InputSource {
public:
    explicit TestInput(std::vector<std::string> lines) : lines_(std::move(lines)) {
    }

    std::string read_line() override {
        if (index_ >= lines_.size()) {
            eof_ = true;
            return "";
        }

        return lines_[index_++];
    }

    bool is_eof() const override {
        return eof_;
    }

private:
    std::vector<std::string> lines_;
    std::size_t index_ = 0;
    bool eof_ = false;
};

class TestOutput : public OutputSink {
public:
    void write(std::string_view text) override {
        output_ += text;
    }

    const std::string& str() const {
        return output_;
    }

private:
    std::string output_;
};

class CapturingSink : public TokenSink {
public:
    void on_chunk(std::string_view chunk) override {
        content += chunk;
    }

    void on_complete() override {
    }

    std::string content;
};

int main() {
    {
        Message m;
        assert(m.role() == Role::System);
        assert(m.content().empty());
    }

    {
        Message m(Role::User, "hello");
        assert(m.role() == Role::User);
        assert(m.content() == "hello");
    }

    {
        Conversation conv;
        assert(conv.size() == 0);
        assert(conv.begin() == conv.end());
    }

    {
        Conversation conv;
        conv.append(Message(Role::System, "be concise"));
        conv.append(Message(Role::User, "hello"));
        conv.append(Message(Role::Assistant, "hi"));

        assert(conv.size() == 3);
        assert(conv.at(0).role() == Role::System);
        assert(conv.at(0).content() == "be concise");
        assert(conv.at(1).role() == Role::User);
        assert(conv.at(1).content() == "hello");
        assert(conv.at(2).role() == Role::Assistant);
        assert(conv.at(2).content() == "hi");

        const Message* it = conv.begin();
        assert((it + 0)->content() == "be concise");
        assert((it + 1)->content() == "hello");
        assert((it + 2)->content() == "hi");
        assert(conv.end() == it + conv.size());
    }

    {
        Conversation a;
        a.append(Message(Role::User, "hello"));
        a.append(Message(Role::Assistant, "world"));

        Conversation b = a;
        assert(a.size() == 2);
        assert(a.begin() != b.begin());
        assert(a.at(0).content() == "hello");
        assert(b.at(0).content() == "hello");
        assert(a.at(1).content() == "world");
        assert(b.at(1).content() == "world");

        b.append(Message(Role::System, "done"));
        assert(a.size() == 2);
        assert(b.size() == 3);
        assert(b.at(2).content() == "done");
    }

    {
        Conversation a;
        a.append(Message(Role::User, "first"));
        a.append(Message(Role::Assistant, "second"));

        Conversation b;
        b = a;
        assert(a.begin() != b.begin());
        assert(a.at(1).content() == "second");
        assert(b.at(1).content() == "second");

        b.append(Message(Role::System, "third"));
        assert(b.at(2).content() == "third");
    }

    {
        Conversation a;
        a.append(Message(Role::User, "first"));

        Conversation b(std::move(a));
        assert(b.size() == 1);
        assert(b.at(0).content() == "first");
        assert(a.size() == 0);
        assert(a.begin() == a.end());

        Conversation c;
        c = std::move(b);
        assert(c.size() == 1);
        assert(c.at(0).content() == "first");
        assert(b.size() == 0);
        assert(b.begin() == b.end());
    }

    {
        Conversation conv;
        conv.append(Message(Role::User, "hello"));
        try {
            conv.at(5);
            assert(false);
        } catch (const std::out_of_range&) {
        }
    }

    {
        Conversation conv;
        conv.append(Message(Role::System, "be concise"));
        conv.append(Message(Role::User, "hello"));
        conv.append(Message(Role::Assistant, "hi"));

        assert(conv.at(0).role() == Role::System);
        assert(conv.at(1).role() == Role::User);
        assert(conv.at(2).role() == Role::Assistant);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out = scanner.feed("Goodbye.<|end_conversation|>");
        assert(out.safe_text == "Goodbye.");
        assert(out.sentinel_found == true);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        std::string text = "Goodbye.<|end_conversation|>";
        for (std::size_t split = 0; split <= text.size(); ++split) {
            SentinelScanner test_scanner("<|end_conversation|>");
            auto first = test_scanner.feed(text.substr(0, split));
            auto second = test_scanner.feed(text.substr(split));
            assert((first.sentinel_found || second.sentinel_found));
            assert(first.safe_text + second.safe_text == "Goodbye.");
        }
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out = scanner.feed("<|end_world|>");
        assert(out.safe_text.empty());
        assert(out.sentinel_found == false);

        auto flushed = scanner.flush();
        assert(flushed.safe_text == "<|end_world|>");
        assert(flushed.sentinel_found == false);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        std::string text;
        for (int i = 0; i < 5000; ++i) {
            text += "<|end_";
        }

        std::string safe_text;
        for (std::size_t i = 0; i < text.size(); i += 7) {
            auto out = scanner.feed(text.substr(i, 7));
            safe_text += out.safe_text;
        }

        auto flushed = scanner.flush();
        safe_text += flushed.safe_text;
        assert(safe_text == text);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out = scanner.feed("Hello");
        assert(out.safe_text.empty());
        assert(out.sentinel_found == false);

        auto flushed = scanner.flush();
        assert(flushed.safe_text == "Hello");
        assert(flushed.sentinel_found == false);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        std::string text = "Goodbye.";
        std::string safe_text;

        for (char ch : text) {
            auto out = scanner.feed(std::string(1, ch));
            safe_text += out.safe_text;
            assert(!out.sentinel_found);
        }

        auto flushed = scanner.flush();
        safe_text += flushed.safe_text;
        assert(safe_text == "Goodbye.");
    }

    {
        std::string path = "/tmp/p2_turn_limit_test.script";
        std::ofstream script(path);
        script << "role: assistant\n";
        script << "I am doing well, thank you! How can I help you?\n";
        script.close();

        TestInput input({"hello"});
        TestOutput output;
        HarnessConfig config;
        config.max_turns = 1;

        Harness harness(std::make_unique<ScriptedModelClient>(path), config);
        auto reason = harness.run(input, output);
        assert(reason.kind == StopReason::Kind::TurnLimit);
        assert(reason.detail == "Max turn limit reached");
    }

    {
        std::string path = "/tmp/p2_sentinel_halt_test.script";
        std::ofstream script(path);
        script << "role: assistant\n";
        script << "Goodbye.<|end_conversation|>\n";
        script.close();

        TestInput input({"hello"});
        TestOutput output;
        HarnessConfig config;
        config.max_turns = 10;

        Harness harness(std::make_unique<ScriptedModelClient>(path), config);
        auto reason = harness.run(input, output);
        assert(reason.kind == StopReason::Kind::Sentinel);
        assert(reason.detail.find("stop sentinel") != std::string::npos);
    }

    {
        std::string path = "/tmp/p2_roundtrip_transcript.txt";
        std::ofstream transcript(path);
        transcript << "role: system\n";
        transcript << "Be concise.\n";
        transcript << "---\n";
        transcript << "role: user\n";
        transcript << "hello\n";
        transcript << "---\n";
        transcript << "role: assistant\n";
        transcript << "Hi! What can I do for you today?\n";
        transcript << "---\n";
        transcript << "role: user\n";
        transcript << "goodbye\n";
        transcript << "---\n";
        transcript << "role: assistant\n";
        transcript << "Goodbye.<|end_conversation|>\n";
        transcript.close();

        ReplayModelClient client(path);
        Conversation conv;

        CapturingSink first;
        client.generate(conv, first);
        assert(first.content == "Hi! What can I do for you today?");

        CapturingSink second;
        client.generate(conv, second);
        assert(second.content == "Goodbye.<|end_conversation|>");
    }

    return 0;
}
