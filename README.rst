==================
vmod_backendhealth
==================

-----------------------------
Varnish Backend Health Module
-----------------------------

:Author: Dag Haavi Finstad
:Date: 2013-12-13
:Version: 1.0
:Manual section: 3

SYNOPSIS
========

import backendhealth;

DESCRIPTION
===========

A vmod that lets you query your Varnish server for a JSON object with
the health status of your backends.

FUNCTIONS
=========

json
----

Prototype
        ::
                json()
Return value
	STRING
Description
	Returns a JSON object with tuples of "backend name": "health status".
Example
        ::
                sub vcl_recv {
                    if (req.url ~ "/backend_health" && client.ip ~ localhost) {
                        error 777 "OK";
                    }
                }
                sub vcl_error {
                    if (obj.status == 777) {
                        set obj.status = 200;
                
                        # JSONP     
                        if (req.url ~ "\?callback=\w+") {
                            set req.http.cb = regsub(req.url, ".*\?callback=(\w+).*", "\1");
                            set obj.http.Content-Type = "application/javascript";
                            synthetic req.http.cb + {"("} + backendhealth.json() + {")"};
                        }
                
                        # JSON
                        else {
                            set obj.http.Content-Type = "application/json";
                            synthetic backendhealth.json();
                        }
                        
                        return (deliver);
                    }
                }

INSTALLATION
============

This is an backendhealth skeleton for developing out-of-tree Varnish
vmods available from the 3.0 release. It implements the "Hello, World!" 
as a vmod callback. Not particularly useful in good hello world 
tradition,but demonstrates how to get the glue around a vmod working.

The source tree is based on autotools to configure the building, and
does also have the necessary bits in place to do functional unit tests
using the varnishtest tool.

Usage::

 ./configure VARNISHSRC=DIR [VMODDIR=DIR]

`VARNISHSRC` is the directory of the Varnish source tree for which to
compile your vmod. Both the `VARNISHSRC` and `VARNISHSRC/include`
will be added to the include search paths for your module.

Optionally you can also set the vmod install directory by adding
`VMODDIR=DIR` (defaults to the pkg-config discovered directory from your
Varnish installation).

Make targets:

* make - builds the vmod
* make install - installs your vmod in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

In your VCL you could then use this vmod along the following lines::
        
        import backendhealth;

        sub vcl_deliver {
                # This sets resp.http.hello to "Hello, World"
                set resp.http.hello = backendhealth.hello("World");
        }

HISTORY
=======

This manual page was released as part of the libvmod-backendhealth package,
demonstrating how to create an out-of-tree Varnish vmod. For further
backendhealths and inspiration check the vmod directory:
https://www.varnish-cache.org/vmods

COPYRIGHT
=========

This document is licensed under the same license as the
libvmod-backendhealth project. See LICENSE for details.

* Copyright (c) 2011 Varnish Software
