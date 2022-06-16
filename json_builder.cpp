#include "json_builder.h"

namespace json
{
	using namespace std::string_literals;

	ItemContext Builder::Value(Node::Value v)
	{
		if (nodes_stack_.empty() && !activated_)
		{
			root_ = v;
			activated_ = true;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && node_dict_value_)
		{
			*node_dict_value_ = v;
			node_dict_value_ = nullptr;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			auto& current_const_array = nodes_stack_.back()->AsArray();
			auto& current_array = const_cast<Array&>(current_const_array);
			current_array.push_back(Node{ v });
		}
		else
		{
			throw std::logic_error("Builder::Value error"s);
		}
		return *this;
	}

	KeyContext Builder::Key(std::string s)
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && !node_dict_value_)
		{
			auto const_dict_node = nodes_stack_.back();
			auto& dict_node = const_cast<Dict&>(const_dict_node->AsMap());
			auto pair_ref = dict_node.insert({ s,Node{} });
			node_dict_value_ = &pair_ref.first->second;
		}
		else
		{
			throw std::logic_error("Builder::Key error"s);
		}
		return *this;
	}

	DictItemContext Builder::StartDict()
	{
		if (nodes_stack_.empty() && !activated_)
		{
			root_ = Node{ Dict{} };
			nodes_stack_.push_back(&root_);
			activated_ = true;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && node_dict_value_)
		{
			*node_dict_value_ = Node{ Dict{} };
			nodes_stack_.push_back(node_dict_value_);
			node_dict_value_ = nullptr;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			auto& current_const_array = nodes_stack_.back()->AsArray();
			auto& current_array = const_cast<Array&>(current_const_array);
			current_array.push_back(Node{ Dict{} });
			nodes_stack_.push_back(&current_array.back());
		}
		else
		{
			throw std::logic_error("Builder::StartDict error"s);
		}
		return *this;
	}

	ArrayItemContext Builder::StartArray()
	{
		if (nodes_stack_.empty() && !activated_)
		{
			root_ = Node{ Array{} };
			nodes_stack_.push_back(&root_);
			activated_ = true;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && node_dict_value_)
		{
			*node_dict_value_ = Node{ Array{} };
			nodes_stack_.push_back(node_dict_value_);
			node_dict_value_ = nullptr;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			auto& current_const_array = nodes_stack_.back()->AsArray();
			auto& current_array = const_cast<Array&>(current_const_array);
			current_array.push_back(Node{ Array{} });
			nodes_stack_.push_back(&current_array.back());
		}
		else
		{
			throw std::logic_error("Builder::StartArray error"s);
		}
		return *this;
	}

	Builder& Builder::EndDict()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap())
		{
			nodes_stack_.pop_back();
		}
		else
		{
			throw std::logic_error("Builder::EndDict error"s);
		}
		return *this;
	}

	Builder& Builder::EndArray()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			nodes_stack_.pop_back();
		}
		else
		{
			throw std::logic_error("Builder::EndArray error"s);
		}
		return *this;
	}

	Node Builder::Build()
	{
		if (!nodes_stack_.empty() || !activated_)
		{
			throw std::logic_error("Builder::Build error"s);
		}
		return root_;
	}

	ItemContext::ItemContext(Builder& b)
		: builder_ref_(b) {}

	KeyContext ItemContext::Key(std::string s)
	{
		return builder_ref_.Key(s);
	}

	ItemContext ItemContext::Value(Node::Value v)
	{
		return builder_ref_.Value(v);
	}

	DictItemContext ItemContext::StartDict()
	{
		return builder_ref_.StartDict();
	}

	ArrayItemContext ItemContext::StartArray()
	{
		return builder_ref_.StartArray();
	}

	Builder& ItemContext::EndDict()
	{
		return builder_ref_.EndDict();
	}

	Builder& ItemContext::EndArray()
	{
		return builder_ref_.EndArray();
	}

	json::Node ItemContext::Build()
	{
		return builder_ref_.Build();
	}


	KeyContext::KeyContext(Builder& b)
		: ItemContext(b) {}

	KeyValueContext KeyContext::Value(Node::Value v)
	{
		return ItemContext::Value(v);
	}


	KeyValueContext::KeyValueContext(Builder& b)
		: ItemContext(b) {}

	KeyValueContext::KeyValueContext(ItemContext b)
		: ItemContext(b) {}


	ArrayValueContext::ArrayValueContext(Builder& b)
		: ItemContext(b) {}

	ArrayValueContext::ArrayValueContext(ItemContext b)
		: ItemContext(b) {}

	ArrayValueContext ArrayValueContext::Value(Node::Value v)
	{
		return ItemContext::Value(v);
	}


	DictItemContext::DictItemContext(Builder& b)
		: ItemContext(b) {}


	ArrayItemContext::ArrayItemContext(Builder& b)
		: ItemContext(b) {}

	ArrayValueContext ArrayItemContext::Value(Node::Value v)
	{
		return ItemContext::Value(v);
	}
}