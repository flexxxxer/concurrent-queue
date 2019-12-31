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
		return this->tail_.load().node == this->head_.load().node;
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
			std::optional<T> value = head.node->value; // get head value

			if (head == this->head_.load()) // if other thread did not change head
			{
				if (value.has_value()) // if value is not dummy (or 'if value exists')
				{
					if (tail != head) // if there is more than one node
					{
						if (first_node_preview.value_or(tag_queue_node<T>()).tag != head.tag) // tags not equal?
						{
							this->fix_list(tail, head); // perform fix list
							continue; // re-iterate dequeue
						}
					}
					else // if there is one node in queue
					{
						queue_node<T>* dummy_node = new queue_node<T>(); // create new dummy node
						dummy_node->next = tag_queue_node<T>(tail.node, tail.tag + 1); // set it's next node

						if (this->tail_.compare_exchange_weak(tail, tag_queue_node<T>(dummy_node, tail.tag + 1))) // try set tail atomically, and if successful
							head.node->preview = tag_queue_node<T>(dummy_node, tail.tag); // set preview
						else // if failed
							delete dummy_node; // free dummy node

						continue; // re-iterate dequeue 
					}
					if (this->head_.compare_exchange_weak(head, tag_queue_node<T>(first_node_preview.value_or(tag_queue_node<T>()).node, head.tag + 1)))
						// try set tail atomically, and if successful
					{
						delete head.node; // free dequeued node
						return value.value(); // dequeue node successful !
					}
				}
				else // if value is dummy (or 'if value not exists')
				{
					if (tail.node == head.node) // tail and head nodes is dummy
						return std::optional<T>(); // empty queue, done!

					if (first_node_preview.value_or(tag_queue_node<T>()).tag != head.tag) // need to skip dummy, and if tags not equal
					{
						this->fix_list(tail, head); // perform fix list
						continue; // re-iterate dequeue
					}

					this->head_.compare_exchange_weak(head, tag_queue_node<T>(first_node_preview.value_or(tag_queue_node<T>()).node, head.tag + 1)); // skip dummy
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

			if (current_node_next.tag != current_node.tag) // tags don’t equal?
				return; // ABA, return! https://en.wikipedia.org/wiki/ABA_problem

			tag_queue_node<T> next_node_preview = current_node_next.node->preview.value_or(tag_queue_node<T>()); // read next node preview

			if (next_node_preview != tag_queue_node<T>(current_node.node, current_node.tag - 1)) // nodes are not equal
				current_node_next.node->preview = tag_queue_node<T>(current_node.node, current_node.tag - 1); // fix

			current_node = tag_queue_node<T>(current_node_next.node, current_node.tag - 1); // advance current node
		}
	}

	std::atomic<tag_queue_node<T>> head_;
	std::atomic<tag_queue_node<T>> tail_;
};