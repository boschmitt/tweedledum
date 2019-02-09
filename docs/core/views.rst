Views
-----

Views can modify the implementation of a network interface by

1. adding interface methods that are not supported by the implementation,
2. changing the implementation of interface methods, and
3. deleting interface methods.

Views implement the network interface and can be passed like a network to an
algorithm. Several views are implemented in tweedledum.

`depth_view`: Compute levels and depth
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Header:** ``tweedledum/views/depth_view.hpp``

.. doxygenclass:: tweedledum::depth_view
   :members:

`immutable_view`: Prevent network changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Header:** ``tweedledum/views/immutable_view.hpp``

.. doxygenclass:: tweedledum::immutable_view
   :members: