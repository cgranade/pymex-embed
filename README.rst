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

In theory, Windows should work as a build target, but ``rebuild_pymex.m`` does not yet automate the
process.

