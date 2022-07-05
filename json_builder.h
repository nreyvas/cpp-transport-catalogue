#pragma once


#include "json.h"

namespace json
{
	class ItemContext;
	class KeyContext;
	class KeyValueContext;
	class ArrayValueContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder
	{
	public:

		KeyContext Key(const std::string& s);
		ItemContext Value(Node::Value v);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;

		Node* node_dict_value_ = nullptr;
		bool activated_ = false;
	};

	class ItemContext
	{
	public:
		ItemContext(Builder& b);

		KeyContext Key(const std::string& s);
		ItemContext Value(Node::Value v);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		json::Node Build();

	protected:
		Builder& builder_ref_;
	};

	class KeyContext : public ItemContext
	{
	public:
		KeyContext(Builder& b);

		KeyValueContext Value(Node::Value v);

		KeyContext Key(const std::string& s) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		json::Node Build() = delete;
	};

	class KeyValueContext : public ItemContext
	{
	public:
		KeyValueContext(Builder& b);
		KeyValueContext(ItemContext b);

		ItemContext Value(Node::Value v) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		json::Node Build() = delete;
	};

	class ArrayValueContext : public ItemContext
	{
	public:
		ArrayValueContext(Builder& b);
		ArrayValueContext(ItemContext b);

		ArrayValueContext Value(Node::Value v);

		KeyContext Key(std::string s) = delete;
		Builder& EndDict() = delete;
		json::Node Build() = delete;
	};

	class DictItemContext : public ItemContext
	{
	public:
		DictItemContext(Builder& b);

		ItemContext Value(Node::Value v) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		json::Node Build() = delete;
	};

	class ArrayItemContext : public ItemContext
	{
	public:
		ArrayItemContext(Builder& b);

		ArrayValueContext Value(Node::Value v);

		KeyContext Key(const std::string& s) = delete;
		Builder& EndDict() = delete;
		json::Node Build() = delete;
	};


}
