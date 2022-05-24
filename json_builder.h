#pragma once

#include "json.h"

namespace json {

class DictItemKeyContext;
class ArrayValueContext;
class DictItemValueContext;

class Builder {
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::vector<std::string> keys_;

public:
    Node& Build();
    Builder& Value(Node::Value val);
    Builder& Key(std::string key);
    DictItemKeyContext StartDict();
    Builder& EndDict();
    ArrayValueContext StartArray();
    Builder& EndArray();

};

template<typename T>
class Context {
public:
    Context(Builder &builder) :
            builder_(builder) {
    }
    ;
    T Value(Node::Value val) {
        return {builder_.Value(std::move(val))};
    }
    T Key(std::string key) {
        return {builder_.Key(std::move(key))};
    }
    auto StartDict() {
        return builder_.StartDict();
    }
    auto EndDict() {
        return builder_.EndDict();
    }
    auto StartArray() {
        return builder_.StartArray();
    }
    auto EndArray() {
        return builder_.EndArray();
    }
private:
    Builder &builder_;
};

class DictItemKeyContext: public Context<DictItemValueContext> {
public:
    DictItemKeyContext(Builder &builder) :
            Context(builder) {
    }
    Context Value(Node::Value val) = delete;
    Context StartDict() = delete;
    Context StartArray() = delete;
    Context EndArray() = delete;

};
class DictItemValueContext: public Context<DictItemKeyContext> {
public:
    DictItemValueContext(Builder &builder) :
            Context(builder) {
    }
    Context Key(std::string key) = delete;
    Context EndDict() = delete;
    Context EndArray() = delete;
private:
};

class ArrayValueContext: public Context<ArrayValueContext> {
public:
    ArrayValueContext(Builder &builder) :
            Context(builder) {
    }
    Context Key(std::string key) = delete;
    Context EndDict() = delete;
};

} // json

