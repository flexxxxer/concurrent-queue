#pragma once

#include "queue_node.h" // use queue_node<T>

// represents queue node with tag
template <typename T>
struct tag_queue_node
{
	queue_node<T>* node; // node, original for every tag queue node
	size_t tag; // id value of node. must be original

	explicit tag_queue_node() : node(nullptr), tag(0) {} // init without node and without tag
	explicit tag_queue_node(queue_node<T>* node, size_t tag) : node(node), tag(tag) {} // init with node and with tag

	bool operator != (const tag_queue_node<T>& node) const
	{
		return node.tag != this->tag || node.node != this->node;
	}
	bool operator == (const tag_queue_node<T>& node) const
	{
		return node.tag == this->tag && node.node == this->node;
	}
};