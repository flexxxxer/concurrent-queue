# dynamic concurrent-queue implementation
dynamic lock-free queue implementation (with optimistic approach)

This implementation was based on `std::atomic<T>` and [this publication](http://people.csail.mit.edu/edya/publications/OptimisticFIFOQueue-journal.pdf)

For use available:
- enqueue value to queue using: `concurrent_queue.enqueue(value)`
- dequeue value from queue using: `auto value = concurrent_queue.dequeue()`
- queue empty information: `auto value = concurrent_stack.is_empty()`

This repo is part of [this project](https://github.com/users/FlexxxerAlex/projects/1).
