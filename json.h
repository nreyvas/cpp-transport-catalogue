#pragma once

#include <istream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>


namespace json
{
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node;

    using Array = std::vector<Node>;
    using Dict = std::map<std::string, Node>;

    class Node {
    public:

        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

        template <typename T>
        Node(T value)
            : value_(std::move(value)) {}
        
        Node();

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const;

        void PushBack(const Node& new_node);

    private:

        Value value_;
    };

    bool operator==(const Node& lhs, const Node& rhs);

    bool operator!=(const Node& lhs, const Node& rhs);

    //----------------------------------------------------------------------------

    class Document
    {
    public:

        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);

    //----------------------------------------------------------------------------

    Node LoadNode(std::istream& input);

    Node LoadNull(std::istream& input);

    Node LoadNumber(std::istream& input);

    std::string LoadString(std::istream& input);

    Node LoadBool(std::istream& input);

    Node LoadArray(std::istream& input);

    Node LoadDict(std::istream& input);

    Document Load(std::istream& input);

    //--------------------------------------------------------------------------
    
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out)
    {
        out << value;
    }

    void PrintValue(bool value, std::ostream& out);

    void PrintValue(std::string s, std::ostream& out);

    void PrintValue(const std::vector<Node>& values, std::ostream& out);

    void PrintValue(std::map<std::string, Node> values, std::ostream& out);

    void PrintValue(std::nullptr_t, std::ostream& out);

    void PrintNode(const Node& node, std::ostream& out);

    void Print(const Document& d, std::ostream& out);
}