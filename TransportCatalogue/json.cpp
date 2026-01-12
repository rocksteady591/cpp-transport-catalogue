#include "json.h"
#include <utility>
#include <cctype>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            if (!(input >> c)) throw ParsingError("Unexpected end of array");
            if (c == ']') return Node(std::move(result));
            input.putback(c);
            while (true) {
                result.push_back(LoadNode(input));
                if (!(input >> c)) throw ParsingError("Unexpected end of array");
                if (c == ']') break;
                if (c != ',') throw ParsingError("Expected ',' or ']'");
            }
            return Node(std::move(result));
        }

        Node LoadString(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == '"') {
                    return Node(std::move(line));
                }
                else if (c == '\\') {
                    input.get(c);
                    switch (c) {
                    case 'r': line += '\r'; break;
                    case 'n': line += '\n'; break;
                    case '"': line += '\"'; break;
                    case '\\': line += '\\'; break;
                    case 't': line += '\t'; break;
                    default: throw ParsingError("Invalid escape sequence");
                    }
                }
                else {
                    line += c;
                }
            }
            throw ParsingError("String parsing error: unclosed string");
        }

        Node LoadDict(std::istream& input) {
            Dict result;
            char c;

            if (!(input >> c)) {
                throw ParsingError("Unexpected end of dict");
            }
            if (c == '}') {
                return Node(std::move(result));
            }

            input.putback(c);

            while (true) {
                if (!(input >> c) || c != '"') {
                    throw ParsingError("Expected key");
                }

                Node key_node = LoadString(input);
                std::string key = key_node.AsString();

                if (!(input >> c) || c != ':') {
                    throw ParsingError("Expected ':'");
                }

                result.emplace_back(
                    std::move(key),
                    LoadNode(input)
                );

                if (!(input >> c)) {
                    throw ParsingError("Unexpected end of dict");
                }
                if (c == '}') {
                    break;
                }
                if (c != ',') {
                    throw ParsingError("Expected ',' or '}'");
                }
            }

            return Node(std::move(result));
        }


        Node LoadBoolOrNull(istream& input) {
            string s;
            while (isalpha(input.peek())) {
                s += static_cast<char>(input.get());
            }
            if (s == "true") return Node(true);
            if (s == "false") return Node(false);
            if (s == "null") return Node(nullptr);
            throw ParsingError("Unknown token: " + s);
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;
            std::string s;

            auto read_char = [&s, &input] {
                s += static_cast<char>(input.get());
                if (!input) throw ParsingError("Failed to read number from stream"s);
                };

            if (input.peek() == '-') read_char();
            while (std::isdigit(input.peek())) read_char();

            bool is_int = true;
            if (input.peek() == '.') {
                is_int = false;
                read_char();
                while (std::isdigit(input.peek())) read_char();
            }

            if (int c = input.peek(); c == 'e' || c == 'E') {
                is_int = false;
                read_char();
                if (input.peek() == '+' || input.peek() == '-') read_char();
                while (std::isdigit(input.peek())) read_char();
            }

            try {
                if (is_int) {
                    try {
                        return Node(std::stoi(s));
                    }
                    catch (...) {
                        return Node(std::stod(s));
                    }
                }
                return Node(std::stod(s));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + s + " to number"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) throw ParsingError("Unexpected end");
            if (c == '[') return LoadArray(input);
            if (c == '{') return LoadDict(input);
            if (c == '"') return LoadString(input);
            if (c == 't' || c == 'f' || c == 'n') {
                input.putback(c);
                return LoadBoolOrNull(input);
            }
            input.putback(c);
            return LoadNumber(input);
        }

    } // namespace

    bool Node::IsInt() const { return std::holds_alternative<int>(value_); }
    bool Node::IsPureDouble() const { return std::holds_alternative<double>(value_); }
    bool Node::IsDouble() const { return IsInt() || IsPureDouble(); }
    bool Node::IsBool() const { return std::holds_alternative<bool>(value_); }
    bool Node::IsString() const { return std::holds_alternative<string>(value_); }
    bool Node::IsNull() const { return std::holds_alternative<nullptr_t>(value_); }
    bool Node::IsArray() const { return std::holds_alternative<Array>(value_); }
    bool Node::IsMap() const { return std::holds_alternative<Dict>(value_); }

    const Array& Node::AsArray() const {
        if (auto* v = std::get_if<Array>(&value_)) return *v;
        throw std::logic_error("Not an array");
    }
    const Dict& Node::AsMap() const {
        if (auto* v = std::get_if<Dict>(&value_)) return *v;
        throw std::logic_error("Not a map");
    }
    int Node::AsInt() const {
        if (auto* v = std::get_if<int>(&value_)) return *v;
        throw std::logic_error("Not an int");
    }
    double Node::AsDouble() const {
        if (auto* v = std::get_if<double>(&value_)) return *v;
        if (auto* v = std::get_if<int>(&value_)) return static_cast<double>(*v);
        throw std::logic_error("Not a double");
    }
    bool Node::AsBool() const {
        if (auto* v = std::get_if<bool>(&value_)) return *v;
        throw std::logic_error("Not a bool");
    }
    const string& Node::AsString() const {
        if (auto* v = std::get_if<string>(&value_)) return *v;
        throw std::logic_error("Not a string");
    }

    Document::Document(Node root) : root_(std::move(root)) {}
    const Node& Document::GetRoot() const { return root_; }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(Node::NodePrinter{ output }, doc.GetRoot().GetValue());
    }

} // namespace json