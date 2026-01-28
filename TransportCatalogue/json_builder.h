#pragma once
#include "json.h"
#include <memory>
#include <string>
#include <vector>

namespace json {
    class DictContext;
    class ArrayContext;
    class KeyContext;
    class EndContext;

    enum class State { Empty, DictKey, DictValue, ArrayValue, Finished };

    class BuilderImpl {
    public:
        void Key(const std::string& key);
        void Value(const json::Node& value);
        void StartDict();
        void StartArray();
        void EndDict();
        void EndArray();
        Node Build() const;
    private:
        State state_ = State::Empty;
        json::Node node_;
        std::vector<Node*> all_nodes_;
        std::string current_key_;
    };

    class KeyContext {
    public:
        explicit KeyContext(std::shared_ptr<BuilderImpl> impl) : impl_(std::move(impl)) {}
        DictContext Value(const Node& value);
        DictContext StartDict();
        ArrayContext StartArray();
    private:
        std::shared_ptr<BuilderImpl> impl_;
    };

    class DictContext {
    public:
        explicit DictContext(std::shared_ptr<BuilderImpl> impl) : impl_(std::move(impl)) {}
        KeyContext Key(const std::string& key);
        EndContext EndDict();
    private:
        std::shared_ptr<BuilderImpl> impl_;
    };

    class ArrayContext {
    public:
        explicit ArrayContext(std::shared_ptr<BuilderImpl> impl) : impl_(std::move(impl)) {}
        ArrayContext Value(const Node& value);
        DictContext StartDict();
        ArrayContext StartArray();
        EndContext EndArray();
    private:
        std::shared_ptr<BuilderImpl> impl_;
    };

    class EndContext {
    public:
        explicit EndContext(std::shared_ptr<BuilderImpl> impl) : impl_(std::move(impl)) {}

        KeyContext Key(const std::string& key);
        DictContext StartDict();
        ArrayContext StartArray();
        EndContext EndDict();
        EndContext EndArray();
        ArrayContext Value(const Node& value);
        Node Build();
    private:
        std::shared_ptr<BuilderImpl> impl_;
    };

    class Builder {
    public:
        Builder() : impl_(std::make_shared<BuilderImpl>()) {}
        KeyContext Key(std::string key);
        EndContext Value(Node value);
        DictContext StartDict();
        ArrayContext StartArray();
        EndContext EndDict();
        EndContext EndArray();
        Node Build();
    private:
        std::shared_ptr<BuilderImpl> impl_;
    };
}