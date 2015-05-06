How to manage your source code using autoconf and automake
http://mij.oltrelinux.com/devel/autoconf-automake/

How to prevent push to master
http://dev.ghost.org/prevent-master-push/

Tips:
  1. remove ITF_Backend/scripts/{Makefile, Makefile.am, Makefile.in} before running 'autoscan'. These files should not be there at beginning.
  2. if config.h.in is missing, try running 'autoreconf --install' before running 'aclocal'