.. image:: doc/logo-cropped.png
    :alt: APEX - Autonomic Performance Environment for eXascale
    :width: 200px

Overview:
=========

One of the key components of the XPRESS project is a new approach to performance observation, measurement, analysis and runtime decision making in order to optimize performance. The particular challenges of accurately measuring the performance characteristics of ParalleX [#]_ applications (as well as other asynchronous multitasking runtime architectures) requires a new approach to parallel performance observation. The standard model of multiple operating system processes and threads observing themselves in a first-person manner while writing out performance profiles or traces for offline analysis will not adequately capture the full execution context, nor provide opportunities for runtime adaptation within OpenX. The approach taken in the XPRESS project is a new performance measurement system, called (Autonomic Performance Environment for eXascale). APEX includes methods for information sharing between the layers of the software stack, from the hardware through operating and runtime systems, all the way to domain specific or legacy applications. The performance measurement components incorporate relevant information across stack layers, with merging of third-person performance observation of node-level and global resources, remote processes, and both operating and runtime system threads.  For a complete academic description of APEX, see the publication "APEX: An Autonomic Performance Environment for eXascale" [#]_.

In short, APEX is an introspection and runtime adaptation library for asynchronous multitasking runtime systems. However, APEX is not *only* useful for AMT/AMR runtimes - it can be used by any application wanting to perform runtime adaptation to deal with heterogeneous and/or variable environments.

Introspection
-------------
APEX provides an API for measuring actions within a runtime. The API includes methods for timer start/stop, as well as sampled counter values. APEX is designed to be integrated into a runtime, library and/or application and provide performance introspection for the purpose of runtime adaptation. While APEX *can* provide rudimentary post-mortem performance analysis measurement, there are many other performance measurement tools that perform that task much better (such as TAU http://tau.uoregon.edu).  That said, APEX includes an event listener that integrates with the TAU measurement system, so APEX events can be forwarded to TAU and collected in a TAU profile and/or trace to be used for post-mortem performance anlaysis.

Runtime Adaptation
------------------
APEX provides a mechanism for dynamic runtime behavior, either for autotuning or adaptation to changing environment.  The infrastruture that provides the adaptation is the Policy Engine, which executes policies either periodically or triggered by events. The policies have access to the performance state as observed by the APEX introspection API. APEX is integrated with Active Harmony (http://www.dyninst.org/harmony) to provide dynamic search for autotuning.

Documentation
=============

Full user documentation is available here: http://khuck.github.io/xpress-apex.

The source code is instrumented with Doxygen comments, and the API reference manual can be generated by executing `make doc` in the build directory, after CMake configuration.  A fairly recent version of the API reference documentation is also available here: http://www.nic.uoregon.edu/~khuck/apex_docs/doc/html/index.html

Installation
============

Full installation documentation is available here: http://khuck.github.io/xpress-apex. Below is a quickstart for the impatient...

APEX is built with CMake. The minimum CMake settings needed for APEX are:

* `-DCMAKE_INSTALL_PREFIX=<some path to an installation location>`
* `-DCMAKE_BUILD_TYPE=<one of Release, Debug, or RelWithDebInfo (recommended)>`

When building for Intel Xeon Phi, Boost may be required if the compiler suite is not up-to-date:

* `-DBOOST_ROOT=<the path to a Boost installation, 1.54 or newer>`

The process for building APEX is:

1) Get the code (assuming v0.5 is the most recent version)::

    wget https://github.com/khuck/xpress-apex/archive/v0.5.tar.gz
    tar -xvzf v0.5.tar.gz

2) Enter the repo directory, make a build directory::

    cd xpress-apex-0.05
    mkdir build
    cd build

3) configure using CMake::

    cmake -DCMAKE_INSTALL_PREFIX=<installation-path> -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

4) build with make::

    make
    make test
    make doc
    make install

Supported Runtime Systems
=========================

HPX (Louisiana State University)
---------------------------------

HPX (High Performance ParalleX) is the original implementation of the ParalleX model. Developed and maintained by the Ste||ar Group at Louisiana State University, HPX is implemented in C++. For more information, see http://stellar.cct.lsu.edu/tag/hpx/.  For a tutorial on HPX with APEX (presented at SC'15, Austin TX) see https://github.com/khuck/SC15_APEX_tutorial.

HPX5 (Indiana University)
-------------------------

HPX-5 (High Performance ParalleX) is a second implementation of the ParalleX model. Developed and maintained by the CREST Group at Indiana University, HPX-5 is implemented in C.  For more information, see https://hpx.crest.iu.edu.

OpenMP
------

The OpenMP API supports multi-platform shared-memory parallel programming in C/C++ and Fortran. The OpenMP API defines a portable, scalable model with a simple and flexible interface for developing parallel applications on platforms from the desktop to the supercomputer.  For more information, see http://openmp.org/.

References
==========
    .. [#] Thomas Sterling, Daniel Kogler, Matthew Anderson, and Maciej Brodowicz. "SLOWER: A performance model for Exascale computing". *Supercomputing Frontiers and Innovations*, 1:42–57, September 2014.  http://superfri.org/superfri/article/view/10
    .. [#] Kevin A. Huck, Allan Porterfield, Nick Chaimov, Hartmut Kaiser, Allen D. Malony, Thomas Sterling, Rob Fowler. "An Autonomic Performance Environment for eXascale", *Journal of Supercomputing Frontiers and Innovations*, 2015.  http://superfri.org/superfri/article/view/64
