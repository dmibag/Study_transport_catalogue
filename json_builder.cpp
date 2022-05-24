#include "json_builder.h"

namespace json {

Node& Builder::Build() {
    if (nodes_stack_.size()) throw std::logic_error("Node is not constructed");
    if (root_.GetValue().index() == 0) throw std::logic_error("Empty Node");
    return root_;
}
Builder& Builder::Value(Node::Value val) {
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    if (nodes_stack_.size()) {
        if (nodes_stack_.back()->IsDict()) {
            Dict &dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            dict.emplace(keys_.back(), std::move(val));
            keys_.pop_back();
        }
        else { // array
            Array *arr = const_cast<Array*>(&nodes_stack_.back()->AsArray());

            if (const auto *v = std::get_if<std::string>(&val)) arr->emplace_back(std::move(*v));
            else if (const auto *v = std::get_if<Dict>(&val)) arr->emplace_back(std::move(*v));
            else if (const auto *v = std::get_if<Array>(&val)) arr->emplace_back(std::move(*v));
            else if (const auto *v = std::get_if<bool>(&val)) arr->emplace_back(*v);
            else if (const auto *v = std::get_if<int>(&val)) arr->emplace_back(*v);
            else if (const auto *v = std::get_if<double>(&val)) arr->emplace_back(*v);
            else arr->emplace_back(nullptr);
        }
    }
    else {
        root_ = std::move(val);
    }
    return *this;
}
Builder& Builder::Key(std::string key) {
    if (nodes_stack_.size() == 0) throw std::logic_error("Dict must be opened");
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    if (!nodes_stack_.back()->IsDict()) throw std::logic_error("Must be a Dict");
    keys_.emplace_back(std::move(key));
    return *this;
}
DictItemKeyContext Builder::StartDict() {
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    nodes_stack_.emplace_back(new Node(std::move(Dict { })));
    return {*this};
}

Builder& Builder::EndDict() {
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    if (nodes_stack_.size() == 0) throw std::logic_error("Dict must be opened");
    Node *node = nodes_stack_.back();
    if (!node->IsDict()) throw std::logic_error("Can't close with dict");
    nodes_stack_.pop_back();

    return Value(std::move(node->AsDict()));
}
ArrayValueContext Builder::StartArray() {
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    nodes_stack_.emplace_back(new Node(std::move(Array { })));
    return {*this};
}

Builder& Builder::EndArray() {
    if (root_.GetValue().index()) throw std::logic_error("Node constructed already");
    if (nodes_stack_.size() == 0) throw std::logic_error("Array must be opened");

    Node *node = nodes_stack_.back();
    if (!node->IsArray()) throw std::logic_error("Can't close with array");
    nodes_stack_.pop_back();

    return Value(std::move(node->AsArray()));
}

}
