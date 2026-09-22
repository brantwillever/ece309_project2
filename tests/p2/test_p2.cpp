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
        conv.append(Message(Role::User, "hello"));
        conv.append(Message(Role::Assistant, "hi"));
        conv.append(Message(Role::System, "be nice"));

        assert(conv.size() == 3);
        assert(conv.at(0).role() == Role::User);
        assert(conv.at(0).content() == "hello");
        assert(conv.at(1).role() == Role::Assistant);
        assert(conv.at(1).content() == "hi");
        assert(conv.at(2).role() == Role::System);
        assert(conv.at(2).content() == "be nice");

        const Message* it = conv.begin();
        assert((it + 0)->content() == "hello");
        assert((it + 1)->content() == "hi");
        assert((it + 2)->content() == "be nice");
        assert(conv.end() == it + conv.size());
    }

    {
        Conversation a;
        a.append(Message(Role::User, "hello"));

        Conversation b = a;
        b.append(Message(Role::Assistant, "world"));

        assert(a.size() == 1);
        assert(a.at(0).content() == "hello");
        assert(b.size() == 2);
        assert(b.at(1).content() == "world");
    }

    {
        Conversation a;
        a.append(Message(Role::User, "first"));
        a.append(Message(Role::Assistant, "second"));

        Conversation b;
        b = a;
        b.append(Message(Role::System, "third"));

        assert(a.size() == 2);
        assert(a.at(1).content() == "second");
        assert(b.size() == 3);
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
    }

    {
        Conversation conv;
        conv.append(Message(Role::User, "hello"));
        try {
            conv.at(5);
            assert(false && "at() should throw on out-of-range access");
        } catch (const std::out_of_range&) {
        }
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out = scanner.feed("Goodbye.<|end_conversation|>");
        assert(out.safe_text == "Goodbye.");
        assert(out.sentinel_found == true);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out1 = scanner.feed("Goodbye.<|end_");
        assert(out1.safe_text == "");
        assert(out1.sentinel_found == false);

        auto out2 = scanner.feed("conversation|>");
        assert(out2.safe_text == "Goodbye.");
        assert(out2.sentinel_found == true);
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        std::string text = "Goodbye.<|end_conversation|>";
        std::string safe_accum;

        for (char ch : text) {
            auto out = scanner.feed(std::string(1, ch));
            if (!out.safe_text.empty()) {
                safe_accum += out.safe_text;
            }
            if (out.sentinel_found) {
                break;
            }
        }

        assert(safe_accum == "Goodbye.");
    }

    {
        SentinelScanner scanner("<|end_conversation|>");
        auto out = scanner.feed("abc<|end_");
        assert(out.safe_text == "");
        assert(out.sentinel_found == false);

        auto flushed = scanner.flush();
        assert(flushed.safe_text == "abc<|end_");
        assert(flushed.sentinel_found == false);
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
        auto out = scanner.feed("<|end_conversation|>tail");
        assert(out.safe_text == "");
        assert(out.sentinel_found == true);
    }

    return 0;
}
