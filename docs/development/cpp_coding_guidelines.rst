C++ Coding Guidelines
=====================

To maintain the code consistent, please use the following conventions. Keep in
mind that from here on "good" and "bad" are used to attribute things that
would make the coding style match, or not match.

.. warning::

   This part of the documentation still a work-in-progess and some old code
   might not follow these guidlines (which is a bug!).


Indentation
-----------

Use **tabs** (and configure your IDE to show a size of 8 spaces for them) for
writing your code (hopefully we can keep this consistent). If you are modifying
someone else’s code, try to keep the coding style similar.


Initialization (=, (), and {})
------------------------------

C++11's uniform initialization solves some syntactical ambiguity in the
language. However, there are cases where ambiguity still exists (for the casual
reader, not the compiler) in what is being called and how. Take the following
code snipet as an example:

.. code-block:: c++

	std::vector<int> vec{42};

What does this do? It could create a `vector<int>` with one hundred 
default-constructed items. Or it could create a `vector<int>` with 1 item whose
value is 42. Both are theoretically possible.

A great overall description of the issue is Herb Sutter’s 
`GotW post <https://herbsutter.com/2013/05/09/gotw-1-solution/>`_.
These guidlines are taken from `Abseil's TotW <https://abseil.io/tips/88>`_.

Guidelines for “How do I initialize a variable?”:

- **Use assignment syntax when initializing directly with literal values.**

.. code-block:: c++

	// Good
	bool error = false;
	std::vector<int> vec = {42};

	// Bad
	bool error{false};
	std::vector<int> vec{42};

- **Use the constructor syntax with parentheses when the initialization is
  performing some active logic.**

.. code-block:: c++

	// Good
	std::vector<int> vec(1, 42);

	// Bad
	std::vector<int> vec{1, 42}; // makes a vector with two integers

- **Use {} initialization without the = only if the above options don’t compile.**
- **Never mix {}s and auto.**