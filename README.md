neeasade's border opt
=============

wmutils-like border programs.

utilities
---------

* chwbb - make 'bevel' style borders
* chwbr - make rounded borders on windows that already have 32-bit color depth
* chbpresel - make preselection indications 

For more information, refer to the programs manpages.

dependencies
------------

Like core, opt depends only on the XCB library.

license
-------

This project and all its code is licensed under the [ISC](http://www.openbsd.org/policy.html)
license. See the LICENSE file.

build & install
---------------

    $ make
    # make install

In the file config.mk you can override build options,
such as the compiler flags or the installation path.
