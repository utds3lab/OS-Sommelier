OS-Sommelier
============

Introduction
------------
A tool for Memory based OS fingerprinting.

Compile
-----------

    cd src
    make

Usage
-----------
The default signature database is OS-Sommelier/md5/

1. Take a snapshot of OS memory first.

2. Signature generating

        ./signa -g [snapshot] 0 > ../md5/OS-name

3. Signature matching

        ./signa -s [snapshot] 0
