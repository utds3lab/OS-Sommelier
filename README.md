OS-Sommelier
============

Introduction
------------

OS-Sommelier is a tool for Memory based OS fingerprinting. It runs on
a 32-bit Linux host. It supports multiple 32-bit guest OSes, like
Linux, Windows, *BSD, etc.

Compile
-----------

    cd src
    make

Usage
-----------
The default signature database is OS-Sommelier/md5/

1. Take a snapshot of guest OS memory first.

2. Signature generating

        ./signa -g [snapshot] 0 > ../md5/OS-name

3. Signature matching

        ./signa -s [snapshot] 0
