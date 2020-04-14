Transformation
***************************

Transform passes mutate a quantum circuit in some simple way that don't fit as optimization.

.. toctree::
   :maxdepth: 2
   :hidden:

   transformation/asap_reschedule
   transformation/remove_marked
   transformation/reverse

+---------------------------+----------------------------------------------------------------------+
| Function                  | Description                                                          |
+===========================+======================================================================+
| :ref:`asap-reschedule`    | As soon as possible (ASAP) rescheduler                               |
+---------------------------+----------------------------------------------------------------------+
| :ref:`remove-marked`      | Remove marked operations.                                            |
+---------------------------+----------------------------------------------------------------------+
| :ref:`reverse`            | Reverse a circuit.                                                   |
+---------------------------+----------------------------------------------------------------------+