CAN-FD Bus 
==========

When simplified, the layout of the CAN bus can be approximated by the following figure.

.. figure:: img/CAN_topology.png

   Simplified layout of the CAN-FD bus.
   
.. note:: Please refer to the :ref:`Hardware design rules section <hwdesignrules>` for information about impedance matching of the CAN-FD bus.


The CAN-FD bus is terminated on both side by `120ohm split-terminations <https://e2e.ti.com/blogs_/b/industrial_strength/posts/the-importance-of-termination-networks-in-can-transceivers>`_.

.. figure:: img/split_termination.png

   Terminator used at each end of the CAN-FD bus.


.. warning::  The standard 2-layer version of RAMN does not feature ESD protection on the CAN bus. On the 4-layer version, an ESD protection diode (`DF3D18FU <https://toshiba.semicon-storage.com/ap-en/semiconductor/product/diodes/tvs-diodes-esd-protection-diodes/detail.DF3D18FU.html>`_) has been added for additional protection.
 
  .. figure:: img/termination_2layer.png
   
    CAN-FD termination on the 2-layer version.

  
  .. figure:: img/termination_4layer.png
   
    CAN-FD termination with ESD protection on the 4-layer version.