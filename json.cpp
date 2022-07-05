#include "json.h"
#include <iostream>

using namespace std;

namespace json
{
    //---------------------- Node methods ----------------------------------------------

    Node::Node(Value v)
        : value_(std::move(v)) {}

    Node::Node() : value_(std::nullptr_t{}) {}

    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const
    {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const
    {
        return std::holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(value_);
    }


    int Node::AsInt() const
    {
        if (IsInt())
        {
            return std::get<int>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not int"s);
        }
    }

    bool Node::AsBool() const
    {
        if (IsBool())
        {
            return std::get<bool>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not bool"s);
        }
    }

    double Node::AsDouble() const
    {
        if (IsInt())
        {
            return (double)std::get<int>(value_);
        }
        else if (IsPureDouble())
        {
            return std::get<double>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not double"s);
        }
    }

    const std::string& Node::AsString() const
    {
        if (IsString())
        {
            return std::get<std::string>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not string"s);
        }
    }

    const Array& Node::AsArray() const
    {
        if (IsArray())
        {
            return std::get<Array>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not Array"s);
        }
    }

    const Dict& Node::AsMap() const
    {
        if (IsMap())
        {
            return std::get<Dict>(value_);
        }
        else
        {
            throw std::logic_error("Node value is not Dict"s);
        }
    }

    const std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>& Node::GetValue() const
    {
        return value_;
    }

    void Node::PushBack(const Node& new_node)
    {
        if (!IsArray())
        {
            throw std::logic_error("Node is not vector, impossible to PushBack"s);
        }
        std::get<Array>(value_).push_back(new_node);
    }

    bool operator==(const Node& lhs, const Node& rhs)
    {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------------------------

    Document::Document(Node root)
        : root_(move(root)) {}

    bool operator==(const Document& lhs, const Document& rhs)
    {
        return (lhs.GetRoot() == rhs.GetRoot());
    }

    bool operator!=(const Document& lhs, const Document& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------------------------

    const Node& Document::GetRoot() const
    {
        return root_;
    }

    Node LoadNode(istream& input)
    {
        char ch;
        if (!(input >> ch))
        {
            throw ParsingError("Empty input"s);
        }
        switch (ch)
        {
        case '[':
            return LoadArray(input);
            break;
        case '{':
            return LoadDict(input);
            break;
        case '"':
            return Node{ LoadString(input) };
            break;
        case 't':
            input.putback(ch);
            return LoadBool(input);
            break;
        case 'f':
            input.putback(ch);
            return LoadBool(input);
            break;
        case 'n':
            input.putback(ch);
            return LoadNull(input);
            break;
        default:
            input.putback(ch);
            return LoadNumber(input);
            break;
        };
    }

    Node LoadNull(std::istream& input)
    {
        std::string null_string;
        char ch;
        for (int i = 0; i < 4; ++i)
        {
            if (input >> ch)
            {
                null_string.push_back(ch);
            }
            else
            {
                throw ParsingError("Bad null parsing"s);
            }
        }
        if (null_string == "null"s)
        {
            return Node{};
        }
        else
        {
            throw ParsingError("Bad null parsing"s);
        }
    }

    Node LoadArray(std::istream& input)
    {
        vector<Node> result;

        char c;
        for (; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (c != ']')
        {
            throw ParsingError("Bad array parsing"s);
        }
        return Node(move(result));
    }

    Node LoadNumber(std::istream& input)
    {
        using namespace std::literals;

        std::string parsed_num;

        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
            {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        auto read_digits = [&input, read_char]
        {
            if (!std::isdigit(input.peek()))
            {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
        {
            read_char();
        }

        if (input.peek() == '0')
        {
            read_char();
        }
        else
        {
            read_digits();
        }

        bool is_int = true;

        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(), ch == '-' || ch == '+')
            {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                try
                {
                    return Node{ std::stoi(parsed_num) };
                }
                catch (...) {}
            }
            return Node{ std::stod(parsed_num) };
        }
        catch (...)
        {
            throw ParsingError("Failed to convert "s + parsed_num + "to number"s);
        }
    }

    std::string LoadString(std::istream& input)
    {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true)
        {
            if (it == end)
            {
                // Stream finished before closing quot met
                throw ParsingError("String parsing error"s);
            }
            const char ch = *it;
            if (ch == '"')
            {
                // Closing quot
                ++it;
                break;
            }
            else if (ch == '\\')
            {
                // Start of an escape sequence met
                ++it;
                if (it == end)
                {
                    // Stream finished right after slash symbol
                    throw ParsingError("String parsing error"s);
                }
                const char escaped_char = *it;
                // Processing one of the sequences: \\, \n, \t, \r, \"
                switch (escaped_char)
                {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('\"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Unrecoginzed escape sequence met
                    throw ParsingError("Unrecoginzed escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                // String literal in JSON cannot be interupted with \r or \n
                throw ParsingError("Unexpected end of line"s);
            }
            else
            {
                s.push_back(ch);
            }
            ++it;
        }

        return s;
    }

    Node LoadBool(std::istream& input)
    {
        std::string bool_string;
        char ch;
        if (input.peek() == 't')
        {
            for (int i = 0; i < 4; ++i)
            {
                if (input >> ch)
                {
                    bool_string.push_back(ch);
                }
                else
                {
                    throw ParsingError("Bad bool parsing"s);
                }
            }
            if (bool_string == "true")
            {
                return Node(true);
            }
            else
            {
                throw ParsingError("Bad bool parsing"s);
            }
        }
        else if (input.peek() == 'f')
        {
            for (int i = 0; i < 5; ++i)
            {
                if (input >> ch)
                {
                    bool_string.push_back(ch);
                }
                else
                {
                    throw ParsingError("Bad bool parsing"s);
                }
            }
            if (bool_string == "false")
            {
                return Node(false);
            }
            else
            {
                throw ParsingError("Bad bool parsing"s);
            }
        }
        else
        {
            throw ParsingError("Bad bool parsing"s);
        }

    }

    Node LoadDict(std::istream& input)
    {
        map<string, Node> result;

        char c;
        for (; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input);
            input >> c;
            result.insert({ move(key), LoadNode(input) });
        }
        if (c != '}')
        {
            throw ParsingError("Bad map parsing"s);
        }
        return Node(move(result));
    }

    Document Load(istream& input)
    {
        return Document{ LoadNode(input) };
    }


    //-------------------------------- Printing functions -------------------------------------

    void PrintContext::PrintIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

    PrintContext PrintContext::Indented() const
    {
        return { out, indent_step, indent_step + indent };
    }

    void PrintValue(std::string s, const PrintContext& ctx)
    {
        std::string result_string{ "\""s };
        for (size_t i = 0; i < s.size(); ++i)
        {
            switch (s[i])
            {
            case '\\':
            {
                if (s[i + 1] == 't')
                {
                    result_string.append("\\\t");
                    ++i;
                }
                else
                {
                    result_string.append("\\\\"s);
                }
                break;
            }
            case '\n':
                result_string.append("\\n"s);
                break;

            case '\r':
                result_string.append("\\r"s);
                break;
            case '\"':
                result_string.append("\\\""s);
                break;
            default:
                result_string.push_back(s[i]);
                break;
            }
        }
        result_string.append("\""s);
        ctx.out << result_string;
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx)
    {
        ctx.out << "null"sv;
    }

    void PrintValue(bool value, const PrintContext& ctx)
    {
        ctx.out << (value ? "true"s : "false"s);
    }

    void PrintValue(double v, const PrintContext& ctx)
    {
        ctx.out << v;
    }

    void PrintValue(int v, const PrintContext& ctx)
    {
        ctx.out << v;
    }

    void PrintValue(const std::vector<Node>& values, const PrintContext& ctx)
    {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool is_first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& n : values)
        {
            if (!is_first)
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(n, inner_ctx);
            is_first = false;
        }
        out.put('\n');
        ctx.PrintIndent();
        out << ']';
    }

    void PrintValue(std::map<std::string, Node> values, const PrintContext& ctx)
    {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool is_first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : values)
        {
            if (!is_first)
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            out.put('\"');
            PrintValue(key, ctx.out);
            out << "\": "sv;
            PrintNode(node, inner_ctx);
            is_first = false;
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx)
    {
        std::visit([&ctx](const auto& value) { PrintValue(value, ctx);  }, node.GetValue());
    }

    void Print(const Document& d, std::ostream& out)
    {
        PrintNode(d.GetRoot(), PrintContext{ out });
    }
}