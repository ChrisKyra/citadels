#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include "user_interface.hpp"

namespace citadels {

    // The interface of the graphical (C#) program. It does not talk to a human but to another
    // program: every call writes ONE compact JSON object on one line and flushes the stream, and
    // the questions then read plain-text answers. The exact format is described in
    // docs/GUI_PROTOCOL.md and tested in tests/test_protocol_ui.cpp.
    class ProtocolUI final : public IUserInterface {
    public:
        ProtocolUI(std::istream& in, std::ostream& out);

        void announce(const std::string& msg) override;
        void handOff(const Player& player) override;
        // Answers are 0-based. A bad answer sends an "error" message and repeats the question.
        int choose(const std::string& prompt, const std::vector<std::string>& options) override;
        bool confirm(const std::string& prompt) override;
        void showTable(const GameState& state) override;
        void showPrivate(const Player& player) override;

    private:
        // Writes the line, a newline, and flushes: the other program waits for it.
        void send(const std::string& line);
        void sendError(const std::string& text);
        // Reads one line without a trailing '\r'; throws std::runtime_error at the end of the input.
        std::string readLine(const char* where);

        std::istream& in_;
        std::ostream& out_;
    };
}
