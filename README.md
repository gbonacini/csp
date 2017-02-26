Description:
============

This tool parse Linux ELF executables checking if they was compiled wit the stack protector feature.

Synopsis:
=========

usage: ./cspp [-v | -vv | -vvv] [-h] file-name

 -v, -vv, -vvv  verbosity level
 -h             print help message 

Return codes:

 0 protected
 1 not protected
 3 error (wrong file type, etc).

Prerequisites:
==============

This program require libelf (libelf-dev).

The program was successfully used with:

- RHEL7 Linux  x86_64;
- Debian 7 ("wheezy");
- Debian 8 ("jessie");
- Ubuntu 16.04.1 LTS
- Ubuntu 16.10

and compiled with: 

- gcc version 6.2.0 20161005 (Ubuntu 6.2.0-5ubuntu12) 
- gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.1) 
- gcc version 4.9.2 (Debian 4.9.2-10) 
- gcc version 4.8.5 20150623 (Red Hat 4.8.5-4) (GCC);


Installation:
=============

- Compile the program:
  make

- Start the program:<BR>
  ./cspp  <efl_file> <BR>
  or  <BR>
  ./cspp [-v | -vv | -vvv ]    # Verbose output, if available

- Online help:

  ./cspp -h

Statistics:
===========

In modern distros, normally, all the "critical" executables should be compiled with the stack protector. Old distros like Debian 7 , strangely, have some executable like sh(dash) and netcat, for example, compiled without the stack protector.
Besides that, plenty of close-source code (i.e VMWare tools ) are compiled without the stack protector.


