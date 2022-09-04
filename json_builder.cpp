#include "json_builder.h"

namespace json {

Builder::ChildValueItemContext Builder::CommonContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder& Builder::CommonContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::ChildDictItemContext Builder::CommonContext::StartDict() {
    return builder_.StartDict();
}

Builder& Builder::CommonContext::EndDict() {
    return builder_.EndDict();
}

Builder::ChildArrayItemContext Builder::CommonContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::CommonContext::EndArray() {
    return builder_.EndArray();
}

Builder::ChildKeyValueItemContext Builder::ChildValueItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::ChildArrayItemValueContext Builder::ChildArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::ChildArrayItemValueContext Builder::ChildArrayItemValueContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::ChildValueItemContext Builder::Key(std::string key) {
    if(nodes_stack_.empty()) {
        throw std::logic_error("error");
    }
    if(!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
        // попытка добавления ключа, если словарь не открывался
        throw std::logic_error("Attempting to add key, but no json::Dict has opened!"); 
    }

    auto [inserted_iterator, is_inserted] = nodes_stack_.back()->AsDict().emplace(key, json::Node{nullptr});

    // идея в том, что открытый ключ кладет на стек указатель на нулевую ноду
    // следующий вызов после ключа - это должен быть Value, Dict, Array
    // если эти вызовы видят на стеке нулевую ноду - они кладут себя тут и закрывают открытую нулевую ноду.
    nodes_stack_.push_back(&(inserted_iterator->second));

    //debug
    //json::Print(json::Document(root_), std::cerr);

    return ChildValueItemContext{*this};
}

Builder& Builder::Value(Node::Value value) {
    if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
        throw std::logic_error("error");
    }
    
    if(!no_content_ && nodes_stack_.empty()) {
        throw std::logic_error("error");         
    }
    
    Node current_node;

    if(std::holds_alternative<std::nullptr_t>(value)) {
        current_node = nullptr;
    } else if(std::holds_alternative<int>(value)) {
        current_node = std::get<int>(value);
    } else if(std::holds_alternative<double>(value)) {
        current_node = std::get<double>(value);
    } else if(std::holds_alternative<std::string>(value)) {
        current_node = std::get<std::string>(value);
    } else if(std::holds_alternative<json::Array>(value)) {
        current_node = std::get<json::Array>(value);
    } else if(std::holds_alternative<json::Dict>(value)) {
        current_node = std::get<json::Dict>(value);
    } else {
        throw std::logic_error("error!");
    }

    if(nodes_stack_.empty()) {
        // Стек пуст - случай, когда json хранит единичное значение.
        root_ = std::move(current_node);
        no_content_ = false;
    } else if(nodes_stack_.back()->IsNull()) {
        // случай когда закрываем открытый ключ
        // это означает, что стек ждет значение для ранее введенного ключа в словаре
        *nodes_stack_.back() = std::move(current_node);
        nodes_stack_.pop_back();
        
    } else if(nodes_stack_.back()->IsArray()) {
        // случай когда добавляем в массив (Array)
        nodes_stack_.back()->AsArray().push_back(std::move(current_node));

    } else {
        throw std::logic_error("error");
    }
    //debug
    // json::Print(json::Document(root_), std::cerr);
    return *this;
}

Builder::ChildDictItemContext Builder::StartDict() {
    return std::get<Builder::ChildDictItemContext>(StartCollection(json::Node{json::Dict{}}));
}

Builder::ChildArrayItemContext Builder::StartArray() {
    return std::get<Builder::ChildArrayItemContext>(StartCollection(json::Node{json::Array{}}));
}

std::variant<Builder::ChildDictItemContext, Builder::ChildArrayItemContext> Builder::StartCollection(json::Node node) {
    if(nodes_stack_.empty() && no_content_) {
        root_ = std::move(node);
        nodes_stack_.emplace_back(&root_);
    } else if(!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
        // случай когда закрываем открытый ключ
        // указатель на нулевую ноду стал указателем на Array
        // далее открыт массив
        *nodes_stack_.back() = std::move(node); // тут
    } else if(!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        // случай когда добавляем в массив (Array) node типа dict
        //nodes_stack_.emplace_back()
        Node inserted_node = nodes_stack_.back()->AsArray().emplace_back(node); // тут
        //в стек помещается адрес только что вставленного словаря
        nodes_stack_.push_back(&inserted_node);

    } else {
        throw std::logic_error("error");
    }        
    //debug
    // json::Print(json::Document(root_), std::cerr);
    if(node.IsArray()) {
        return ChildArrayItemContext{*this}; 
    } else if (node.IsDict()) {
        return ChildDictItemContext{*this};
    } else {
        throw std::logic_error("bad node in return Builder::StartCollection().");
    }
}

Builder& Builder::EndDict() {
    if(!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
        // попытка добавления ключа, если словарь не открывался
        throw std::logic_error("error"); 
    }
    nodes_stack_.pop_back();
    no_content_ = false;
    //debug
    // json::Print(json::Document(root_), std::cerr);
    return *this;
}

Builder& Builder::EndArray(){
    if(!nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
        // попытка добавления ключа, если словарь не открывался
        throw std::logic_error("error"); 
    }
    nodes_stack_.pop_back();  
    no_content_ = false; 
    //debug
    // json::Print(json::Document(root_), std::cerr);
    return *this;
}

Node Builder::Build() {
    if(!nodes_stack_.empty()) {
        throw std::logic_error("error");
    }

    if(no_content_) {
        throw std::logic_error("error");
    }        
    //debug
    // json::Print(json::Document(root_), std::cerr);
    return root_;
}

} // namespace json