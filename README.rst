=====
pymex
=====

**pymex** embeds the Python runtime (currently only works for version 2.7) into the MATLAB environment,
using the MEX API. This allows MATLAB programs to leverage Python-based functionality by evaluating,
importing and manipulating Python modules.

Currently, **pymex** should be thought of as alpha, as it may cause MATLAB to crash hard.
Such crashes are most likely due to missing or excess ``Py_XDECREF`` calls, as such errors
currently propagate outward as segfaults.

Note that **pymex** also is 64-bit only at the moment, due to hardcoding the size of Python pointers
for marshalling to MATLAB data structures. This limitation can be removed, but is currently not a
high priority.

Installation Instructions
-------------------------

The following instructions courtesy of Ian Hincks.

Linux/Unix
~~~~~~~~~~

1) Make sure ``python-dev`` or the equivalent is installed on your computer. You will need ``Python.h``.
2) Make sure g++ is installed. On Ubuntu:
    $ sudo apt-get install g++
3) Run rebuild_pymex.m in MATLAB. 

Windows
~~~~~~~

1) Make sure you have Python 2.7 (64-bit) installed. `Enthought Python
   Distribution 7.3`_ has been tested, but other distributions may also
   work.
2) Install `Windows SDK 7.1`_ or any other compiler supported by MEX.
   You can check that you have setup MEX correctly by running::

    >> mex -setup

3) Run rebuild_pymex.m in MATLAB.

    >> rebuild_pymex

Known Issues
------------

All Platforms
~~~~~~~~~~~~~

* After calling ``str`` (see ``test_case.m``), running ``clear all``
  hard crashes MATLAB.

Windows
~~~~~~~

* Output sent to Python ``sys.stdout`` is not visible from within the
  MATLAB Desktop.

.. _Enthought Python Distribution 7.3: https://www.enthought.com/
.. _Windows SDK 7.1: http://www.microsoft.com/en-us/download/details.aspx?id=8279