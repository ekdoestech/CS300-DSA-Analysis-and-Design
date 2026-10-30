## CS 300 Project Reflection

In these projects, I developed a course advising program that reads course records, displays them in alphanumeric order, and lets an advisor look up a course and its prerequisites. The work challenged me to choose data structures that fit the program’s operations while considering runtime and memory use.

I compared vectors, hash tables, and binary search trees because each handles storage and lookup differently. A vector is simple to use, but searching it requires checking records one at a time. A hash table can make lookups efficient when collisions are limited, while a binary search tree supports course lookup and ordered output. Comparing these approaches helped me see why understanding data structures matters: the choice affects how efficiently a program can perform as its data grows.

One roadblock was making sure course records and prerequisites were valid. I had to think through what should happen when input is malformed, a course is missing, or a prerequisite cannot be found. I also learned that a binary search tree’s performance depends on its shape. An unbalanced tree can slow lookups, so you need to weigh a structure's theoretical benefits against its limitations.

These projects changed how I approach program design. I now think about a program's operations before choosing how to store its data. I also consider how to validate input, handle errors, and present results, rather than focusing only on the main algorithm.

I used separate functions and a `Course` structure to keep responsibilities organized. Clear names, comments for non-obvious decisions, and input checks make the program easier to understand and maintain. Comparing multiple data structures also helped me consider how the program could adapt if requirements or data volume changed.
