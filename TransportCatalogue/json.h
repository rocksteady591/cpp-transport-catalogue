#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

namespace json {

    class Node;
    using Dict = std::vector<std::pair<std::string, Node>>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        // ����� ������������� ��� ���������� maybe-uninitialized
        Node() : value_(nullptr) {}

        explicit Node(NodeValue value) : value_(std::move(value)) {}
        Node(Array array) : value_(std::move(array)) {}
        Node(Dict map) : value_(std::move(map)) {}
        Node(int value) : value_(value) {}
        Node(bool value) : value_(value) {}
        Node(const char* value) : value_(std::string(value)) {}
        Node(std::string value) : value_(std::move(value)) {}
        Node(std::nullptr_t value) : value_(value) {}
        Node(double value) : value_(value) {}

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        // ������� �� ������ � ���������� ����� ��� ����������� ������ ����������
        const NodeValue& GetValue() const {
            return value_;
        }

        // ��������� ��������� ��� C++17
        bool operator==(const Node& other) const {
            return value_ == other.value_;
        }
        bool operator!=(const Node& other) const {
            return !(*this == other);
        }

        struct NodePrinter {
            std::ostream& out;

            void operator()(std::nullptr_t) const { out << "null"; }
            void operator()(bool b) const { out << (b ? "true" : "false"); }
            void operator()(int i) const { out << i; }
            void operator()(double d) const { out << d; }
            void operator()(const std::string& s) const {
                out << '\"';
                for (char ch : s) {
                    switch (ch) {
                    case '\"': out << "\\\""; break;
                    case '\\': out << "\\\\"; break;
                    case '\t': out << "\\t"; break;
                    case '\r': out << "\\r"; break;
                    case '\n': out << "\\n"; break;
                    default: out << ch; break;
                    }
                }
                out << '\"';
            }
            void operator()(const Array& arr) const {
                out << '[';
                bool first = true;
                for (const auto& node : arr) {
                    if (!first) out << ", ";
                    std::visit(*this, node.GetValue());
                    first = false;
                }
                out << ']';
            }
            void operator()(const Dict& dict) const {
                out << '{';
                bool first = true;
                for (const auto& [key, value] : dict) {
                    if (!first) out << ", ";
                    (*this)(key); // �������� ���� ��� ������
                    out << ": ";
                    std::visit(*this, value.GetValue());
                    first = false;
                }
                out << '}';
            }
        };

    private:
        NodeValue value_;
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;

        // �������� ��� ������:
        bool operator==(const Document& other) const {
            return root_ == other.root_;
        }
        bool operator!=(const Document& other) const {
            return !(*this == other);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);

}  // namespace json#pragma once
