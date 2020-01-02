#pragma once

#include "queue data structs/tag_queue_node.h" // use tag_queue_node and queue node
#include <atomic> // use std::atomic<T>

template <typename T>
class concurrent_queue
{
public:

	concurrent_queue()
	{
		// create dummy node
		tag_queue_node<T> dummy_tqn = tag_queue_node<T>(new queue_node<T>, 0);

		this->head_.store(dummy_tqn); // set head as dummy node
		this->tail_.store(dummy_tqn); // set head as dummy node
	}

	[[nodiscard]] bool is_empty() const noexcept
	{
		return this->tail_.load() == this->head_.load(); // if nodes are equal
	}

	void enqueue(const T& value) noexcept
	{
		queue_node<T>* new_node = new queue_node<T>(value); // create new node

		while (true) // do till success
		{
			tag_queue_node<T> tail = this->tail_.load(); // get the tail atomically
			new_node->next = tag_queue_node<T>(tail.node, tail.tag + 1); // set node's next queue node

			if (this->tail_.compare_exchange_weak(tail, tag_queue_node<T>(new_node, tail.tag + 1))) // try set tail atomically, and if successful
			{
				tail.node->preview = tag_queue_node<T>(new_node, tail.tag); // then write preview queue node
				break; // enqueue was done
			}
		}
	}
	std::optional<T> dequeue() noexcept
	{
		while (true) // try till success or empty
		{
			tag_queue_node<T> head = this->head_.load(); // get the head atomically
			tag_queue_node<T> tail = this->tail_.load(); // get the tail atomically
			std::optional<tag_queue_node<T>> first_node_preview = head.node->preview; // get first node preview node

			if (head == this->head_.load()) // if other thread did not change head
			{
				if (tail != head) // if there is more than one node
				{
					if (first_node_preview.value_or(tag_queue_node<T>()).tag != head.tag) // tags not equal?
					{
						this->fix_list(tail, head); // perform fix list
						continue; // re-iterate dequeue
					}

					queue_node<T>* node_preview_node = first_node_preview.value_or(tag_queue_node<T>()).node;
					std::optional<T> value = node_preview_node == nullptr ? std::optional<T>() : node_preview_node->value; // read value

					if (this->head_.compare_exchange_weak(head, tag_queue_node<T>(first_node_preview.value_or(tag_queue_node<T>()).node, head.tag + 1)))
						// try set head atomically, and if successful
					{
						delete head.node; // free dequeued node
						return value; // dequeue node successful !
					}
				}
				else // only one node
				{
					return std::optional<T>(); // queue is empty
				}
			}
		}
	}

private:

	// to fix ABA Problem
	void fix_list(tag_queue_node<T> tail, tag_queue_node<T> head)
	{
		tag_queue_node<T> current_node = tail; // init current node as tail

		while (head == this->head_.load() && current_node != head) // while not at head
		{
			tag_queue_node<T> current_node_next = current_node.node->next.value_or(tag_queue_node<T>()); // read read current node next
			current_node_next.node->preview = tag_queue_node<T>(current_node.node, current_node.tag - 1); // fix
			current_node = tag_queue_node<T>(current_node_next.node, current_node.tag - 1); // advance current node
		}
	}

	std::atomic<tag_queue_node<T>> head_;
	std::atomic<tag_queue_node<T>> tail_;
};