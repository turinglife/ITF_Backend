How to manage your source code using autoconf and automake
http://mij.oltrelinux.com/devel/autoconf-automake/

How to prevent push to master
http://dev.ghost.org/prevent-master-push/

Tips:
  1. remove ITF_Backend/scripts/{Makefile, Makefile.am, Makefile.in} before running 'autoscan'. These files should not be there at beginning.
  2. if config.h.in is missing, try running 'autoreconf --install' before running 'aclocal'

###Mult-Task Test on TITAN (Testing by Shenzhen Sensetime)
Dataset: 200608

Note: The network is initialised for each task separately

Analyser Num | Patch Num | Counting FPS | Memory Used
:---: | :---: | :---: | :---:
1 | 171 | 11 | about 320 MB
6 | 171 | 2 | about 2000 MB

