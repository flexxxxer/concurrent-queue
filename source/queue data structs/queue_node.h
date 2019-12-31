#pragma once

#include <optional> // use std::optional

template <typename T>
struct tag_queue_node;

// represents queue node
template <typename T>
struct queue_node
{
	std::optional<T> value; // contained value. can be dummy (no have value)

	std::optional<tag_queue_node<T>> next; // next node, may be absent
	std::optional<tag_queue_node<T>> preview; // preview node, may be absent (the node for solving the ABA Problem)

	explicit queue_node() : value(std::optional<T>()), next(std::optional<tag_queue_node<T>>()), preview(std::optional<tag_queue_node<T>>()) {} // init without value
	explicit queue_node(T value) : value(std::optional<T>(value)), next(std::optional<tag_queue_node<T>>()), preview(std::optional<tag_queue_node<T>>()) {} // init with value
	explicit queue_node(T value, tag_queue_node<T> next, tag_queue_node<T> preview) : value(std::optional<T>(value)), next(std::optional<tag_queue_node<T>>(next)), preview(std::optional<tag_queue_node<T>>(preview)) {} // init with value, next and preview nodes
};