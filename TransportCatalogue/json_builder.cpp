#include "json_builder.h"
#include <stdexcept>

namespace json {

        void BuilderImpl::Key(const std::string& key) {
        if (state_ != State::DictKey) throw std::logic_error("Key error");
        state_ = State::DictValue;
        current_key_ = key;
    }

    void BuilderImpl::Value(const json::Node& value) {
        if (state_ != State::ArrayValue && state_ != State::DictValue && state_ != State::Empty)
            throw std::logic_error("Value error");

        if (state_ == State::DictValue) {
            all_nodes_.back()->AsMap().emplace_back(current_key_, value);
            state_ = State::DictKey;
        }
        else if (state_ == State::ArrayValue) {
            all_nodes_.back()->AsArray().push_back(value);
        }
        else {
            node_ = value;
            state_ = State::Finished;
        }
    }

    void BuilderImpl::StartDict() {
        if (state_ != State::Empty && state_ != State::ArrayValue && state_ != State::DictValue)
            throw std::logic_error("StartDict error");

        Node n{ Dict{} };
        if (state_ == State::Empty) {
            node_ = std::move(n);
            all_nodes_.push_back(&node_);
        }
        else if (state_ == State::ArrayValue) {
            all_nodes_.back()->AsArray().push_back(std::move(n));
            all_nodes_.push_back(&all_nodes_.back()->AsArray().back());
        }
        else {
            all_nodes_.back()->AsMap().emplace_back(current_key_, std::move(n));
            all_nodes_.push_back(&all_nodes_.back()->AsMap().back().second);
        }
        state_ = State::DictKey;
    }

    void BuilderImpl::StartArray() {
        if (state_ != State::Empty && state_ != State::ArrayValue && state_ != State::DictValue)
            throw std::logic_error("StartArray error");

        Node n{ Array{} };
        if (state_ == State::Empty) {
            node_ = std::move(n);
            all_nodes_.push_back(&node_);
        }
        else if (state_ == State::ArrayValue) {
            all_nodes_.back()->AsArray().push_back(std::move(n));
            all_nodes_.push_back(&all_nodes_.back()->AsArray().back());
        }
        else {
            all_nodes_.back()->AsMap().emplace_back(current_key_, std::move(n));
            all_nodes_.push_back(&all_nodes_.back()->AsMap().back().second);
        }
        state_ = State::ArrayValue;
    }

    void BuilderImpl::EndDict() {
        if (all_nodes_.empty() || !all_nodes_.back()->IsMap()) throw std::logic_error("EndDict error");
        all_nodes_.pop_back();
        if (all_nodes_.empty()) state_ = State::Finished;
        else state_ = all_nodes_.back()->IsMap() ? State::DictKey : State::ArrayValue;
    }

    void BuilderImpl::EndArray() {
        if (all_nodes_.empty() || !all_nodes_.back()->IsArray()) throw std::logic_error("EndArray error");
        all_nodes_.pop_back();
        if (all_nodes_.empty()) state_ = State::Finished;
        else state_ = all_nodes_.back()->IsMap() ? State::DictKey : State::ArrayValue;
    }

    Node BuilderImpl::Build() const {
        if (state_ != State::Finished) throw std::logic_error("Build error: not finished");
        return node_;
    }

   // Builder
    KeyContext Builder::Key(std::string key) { impl_->Key(key); return KeyContext(impl_); }
    EndContext Builder::Value(Node value) { impl_->Value(value); return EndContext(impl_); }
    DictContext Builder::StartDict() { impl_->StartDict(); return DictContext(impl_); }
    ArrayContext Builder::StartArray() { impl_->StartArray(); return ArrayContext(impl_); }
    Node Builder::Build() { return impl_->Build(); }
    EndContext Builder::EndDict(){impl_->EndDict();return EndContext(impl_);}
    EndContext Builder::EndArray(){impl_->EndArray();return EndContext(impl_);}

    // KeyContext
    DictContext KeyContext::Value(const Node& value) { impl_->Value(value); return DictContext(impl_); }
    DictContext KeyContext::StartDict() { impl_->StartDict(); return DictContext(impl_); }
    ArrayContext KeyContext::StartArray() { impl_->StartArray(); return ArrayContext(impl_); }

    // DictContext
    KeyContext DictContext::Key(const std::string& key) { impl_->Key(key); return KeyContext(impl_); }
    EndContext DictContext::EndDict() { impl_->EndDict(); return EndContext(impl_); }

    // ArrayContext
    ArrayContext ArrayContext::Value(const Node& value) { impl_->Value(value); return *this; }
    DictContext ArrayContext::StartDict() { impl_->StartDict(); return DictContext(impl_); }
    ArrayContext ArrayContext::StartArray() { impl_->StartArray(); return ArrayContext(impl_); }
    EndContext ArrayContext::EndArray() { impl_->EndArray(); return EndContext(impl_); }

    // EndContext
    KeyContext EndContext::Key(const std::string& key) { impl_->Key(key); return KeyContext(impl_); }

    // EndContext
    DictContext EndContext::StartDict() { impl_->StartDict(); return DictContext(impl_); }
    ArrayContext EndContext::StartArray() { impl_->StartArray(); return ArrayContext(impl_); }

    EndContext EndContext::EndDict() { impl_->EndDict(); return EndContext(impl_); }
    EndContext EndContext::EndArray() { impl_->EndArray(); return EndContext(impl_); }
    ArrayContext EndContext::Value(const Node& value) { impl_->Value(value); return ArrayContext(impl_); }
    Node EndContext::Build() { return impl_->Build(); }

} // namespace json