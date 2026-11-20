#include <ciph/platform/windows_command_line.hpp>

namespace ciph::platform {

std::string
quote_windows_argument(std::string_view argument) {
    // Nothing that the parser treats specially: pass it through untouched.
    if (!argument.empty() && argument.find_first_of(" \t\n\v\"") == std::string_view::npos) {
        return std::string(argument);
    }

    // Otherwise wrap it in quotes. Inside quotes the parser's rules are:
    //   - a backslash is literal, unless it is part of a run of backslashes
    //     that ends at a quote;
    //   - 2n backslashes followed by a quote mean n backslashes and the quote
    //     ends the argument;
    //   - 2n+1 backslashes followed by a quote mean n backslashes and a
    //     literal quote.
    // So backslashes only need doubling when a quote follows them, and that
    // includes the closing quote added at the end.
    std::string quoted = "\"";
    std::size_t backslashes = 0;

    for (char c : argument) {
        if (c == '\\') {
            ++backslashes;
            continue;
        }
        if (c == '"') {
            // Double the run, then one more backslash to escape the quote.
            quoted.append(backslashes * 2 + 1, '\\');
        } else {
            // The run is not followed by a quote, so it stays as it is.
            quoted.append(backslashes, '\\');
        }
        quoted.push_back(c);
        backslashes = 0;
    }

    // A trailing run is followed by the closing quote: double it so the quote
    // still closes the argument.
    quoted.append(backslashes * 2, '\\');
    quoted.push_back('"');
    return quoted;
}

std::string
windows_command_line(std::string_view executable, const std::vector<std::string>& arguments) {
    std::string command_line = quote_windows_argument(executable);
    for (const std::string& argument : arguments) {
        command_line.push_back(' ');
        command_line += quote_windows_argument(argument);
    }
    return command_line;
}

} // namespace ciph::platform
