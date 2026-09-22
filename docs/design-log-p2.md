# Design Log — Project 2

(500-800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost

For 'Conversation:append()', I used a doubling strategy to ensure that whenever the buffer becomes full, the capacity doubles itself. I also made sure that the capacity begins at 1 so that there is a capacity to be doubled in the first place. I did this because we went over in class that generally, it is most proper to double the size of a buffer whenever an expansion is necessary.

If the conversation has 'n' elements and the buffer is at a capacity of 'n' (the buffer is full), then the next expansion will have a complexity of O(n), since it has to copy all of the existing elements into a new array. However, my doubling strategy ensures that this operation takes place as little as possible, with it only being necessary at sizes 1, 2, 4, 8, 16, and any following powers of 2.

Given that 'N' is the final number of elements, the total cost of all expansions up to size 'N' is bounded by the equation:
1 + 2 + 4 + ... + 2^k = O(2^k) = O(N)

The cost of each individual append is amortized constant time, since the expansion work is spread out over many appends, making it less memory-intensive due to it being spread out. The average cost per insert still remains O(1).

## Rule of Five evidence

Because the 'Conversation' class owns a heap-allocated array of 'Message' objects, it must follow the Rule of Five.

The destructor class deletes the array and resets the object. The copy constructor class performs a deep copy, allocating a new array and copying each 'Message' from the source array. The assignment operator class generally does the same thing as the copy constructor, except it also protects against self-assignment and contains a temporary buffer in order to prevent deleting the original copy before the new data is properly copied. The move constructor class takes the pointer from the source array and sets the source's fields to zero. The move assignment operator does basically the same thing. This is a proper implementation of move semantics because it allows the source object to remain valid and able for the destructor to delete the source array without causing any memory bugs.

## Sentinel scanner: bounded pending_ proof

The sentinel scanner uses pending_text as a buffer of trailing characters that may be the beginning of the sentinel. If we assume that k = sentinel.size() - 1, then every time feed() is called, it inspects pending_text and chunk. If the entire combined string does not contain the sentinel, then all but the last k characters are safe to immediately ignore, and the last k characters are then kept as the new pending_text.

This implementation ensures that pending_text will never contain more than k characters, because after every call of feed(), any characters that come before the final k characters are discarded. This means that the amount of withheld state is held by a constant that solely depends on the sentinel length, giving us O(1) extra memory per chunk and avoiding any erratic behavior that would come from scanning the string from scratch each iteration.

## What I would change differently

I think that if I had more time to work on this project, and if I had more knowledge about C++ programming and especially test cases going into this project, I would have implemented most of my code differently, especially with the sentinel and with my test cases. Developing my test cases took a lot of guesswork and trial-by-error, and it took me a long time to figure out. I also feel like there is a more efficient implementation I could have used for the sentinel scanner that with more time, I could have figured out. However, I do think that my current implementation works well enough.