# Time-stamp: <2014-02-21 11:51:05 cs3612>

#rm outData
#rm outTime
##rm PTEDATA
#rm outDataPgd

./signa -g ~/qemu/mem/mem-winxp 0 > ../md5/Winxp
echo "# ./signa -g ~/qemu/mem/mem-winxp 0 > ../md5/Winxp"
./signa -g ~/qemu/mem/mem-winxpsp2-b 0 > ../md5/Win-XP-sp2
echo "# ./signa -g ~/qemu/mem/mem-winxpsp2-b 0 > ../md5/Win-XP-sp2"
./signa -g ~/qemu/mem/mem-winxpsp3-a 0 > ../md5/Win-XP-sp3
./signa -g ~/qemu/mem/mem-winvista 0 > ../md5/Win-Vista
./signa -g ~/qemu/mem/mem-win7-b 0 > ../md5/Win-7
./signa -g ~/qemu/mem/mem-win2k 0 > ../md5/Win-2k-Server
./signa -g ~/qemu/mem/mem-win2003 0 > ../md5/Win2003
./signa -g ~/qemu/mem/mem-win2003sp2 0 > ../md5/Win2003sp2
./signa -g ~/qemu/mem/mem-win2008 0 > ../md5/Win2008
./signa -g ~/qemu/mem/mem-win2008sp2 0 > ../md5/Win2008sp2
./signa -g ~/qemu/mem/mem-freebsd7.4 0 > ../md5/FreeBSD-7.4 
./signa -g ~/qemu/mem/mem-freebsd8.0 0 > ../md5/FreeBSD-8.0
./signa -g ~/qemu/mem/mem-freebsd8.2 0 > ../md5/FreeBSD-8.2
./signa -g ~/qemu/mem/mem-freebsd8.3 0 > ../md5/FreeBSD-8.3
echo "# ./signa -g ~/qemu/mem/mem-freebsd8.3 0 > ../md5/FreeBSD-8.3"
./signa -g ~/qemu/mem/mem-freebsd9 0 > ../md5/FreeBSD-9.0
./signa -g ~/qemu/mem/mem-openbsd4.7 0 > ../md5/OpenBSD-4.7
./signa -g ~/qemu/mem/mem-openbsd4.8 0 > ../md5/OpenBSD-4.8
./signa -g ~/qemu/mem/mem-openbsd4.9 0 > ../md5/OpenBSD-4.9
./signa -g ~/qemu/mem/mem-openbsd5 0 > ../md5/OpenBSD-5
./signa -g ~/qemu/mem/mem-openbsd5.1 0 > ../md5/OpenBSD-5.1
echo "# ./signa -g ~/qemu/mem/mem-openbsd5.1 0 > ../md5/OpenBSD-5.1"
./signa -g ~/qemu/mem/mem-netbsd4.0 0 > ../md5/NetBSD-4.0
echo "# ./signa -g ~/qemu/mem/mem-netbsd4.0 0 > ../md5/NetBSD-4.0"
./signa -g ~/qemu/mem/mem-netbsd4.0.1 0 > ../md5/NetBSD-4.0.1
echo  "# ./signa -g ~/qemu/mem/mem-netbsd4.0.1 0 > ../md5/NetBSD-4.0.1"
./signa -g ~/qemu/mem/mem-netbsd5.0 0 > ../md5/NetBSD-5.0
echo "# ./signa -g ~/qemu/mem/mem-netbsd5.0 0 > ../md5/NetBSD-5.0"
./signa -g ~/qemu/mem/mem-netbsd5.0.1 0 > ../md5/NetBSD-5.0.1
echo '# ./signa -g ~/qemu/mem/mem-netbsd5.0.1 0 > ../md5/NetBSD-5.0.1'
./signa -g ~/qemu/mem/mem-netbsd5.0.2 0 > ../md5/NetBSD-5.0.2
echo '# ./signa -g ~/qemu/mem/mem-netbsd5.0.2 0 > ../md5/NetBSD-5.0.2'
./signa -g ~/qemu/mem/mem-netbsd5.1 0 > ../md5/NetBSD-5.1
echo '# ./signa -g ~/qemu/mem/mem-netbsd5.1 0 > ../md5/NetBSD-5.1'
./signa -g ~/qemu/mem/mem-netbsd5.1.2 0 > ../md5/NetBSD-5.1.2
echo '# ./signa -g ~/qemu/mem/mem-netbsd5.1.2 0 > ../md5/NetBSD-5.1.2'

#Linux
./signa -g ~/qemu/mem/mem-2.6.26 0 > ../md5/Linux-2.6.26
./signa -g ~/qemu/mem/mem-2.6.27 0 > ../md5/Linux-2.6.27
./signa -g ~/qemu/mem/mem-2.6.28 0 > ../md5/Linux-2.6.28
./signa -g ~/qemu/mem/mem-2.6.28.1 0 > ../md5/Linux-2.6.28.1
./signa -g ~/qemu/mem/mem-2.6.28.2 0 > ../md5/Linux-2.6.28.2
./signa -g ~/qemu/mem/mem-2.6.29 0 > ../md5/Linux-2.6.29
./signa -g ~/qemu/mem/mem-2.6.30 0 > ../md5/Linux-2.6.30
./signa -g ~/qemu/mem/mem-2.6.31 0 > ../md5/Linux-2.6.31
echo "./signa -g ~/qemu/mem/mem-2.6.31 0 > ../md5/Linux-2.6.31"
./signa -g ~/qemu/mem/mem-2.6.32.27 0 > ../md5/Linux-2.6.32.27
./signa -g ~/qemu/mem/mem-2.6.33 0 > ../md5/Linux-2.6.33
./signa -g ~/qemu/mem/mem-2.6.34 0 > ../md5/Linux-2.6.34
./signa -g ~/qemu/mem/mem-2.6.35 0 > ../md5/Linux-2.6.35
echo './signa -g ~/qemu/mem/mem-2.6.35 0 > ../md5/Linux-2.6.35'
./signa -g ~/qemu/mem/mem-2.6.36 0 > ../md5/Linux-2.6.36
echo './signa -g ~/qemu/mem/mem-2.6.36 0 > ../md5/Linux-2.6.36'
./signa -g ~/qemu/mem/mem-2.6.36.1 0 > ../md5/Linux-2.6.36.1
./signa -g ~/qemu/mem/mem-2.6.36.2 0 > ../md5/Linux-2.6.36.2
./signa -g ~/qemu/mem/mem-2.6.36.3 0 > ../md5/Linux-2.6.36.3
./signa -g ~/qemu/mem/mem-2.6.36.4 0 > ../md5/Linux-2.6.36.4
echo './signa -g ~/qemu/mem/mem-2.6.36.4 0 > ../md5/Linux-2.6.36.4'
./signa -g ~/qemu/mem/mem-2.6.37 0 > ../md5/Linux-2.6.37
./signa -g ~/qemu/mem/mem-2.6.37.1 0 > ../md5/Linux-2.6.37.1
./signa -g ~/qemu/mem/mem-2.6.37.2 0 > ../md5/Linux-2.6.37.2
./signa -g ~/qemu/mem/mem-2.6.37.3 0 > ../md5/Linux-2.6.37.3
./signa -g ~/qemu/mem/mem-2.6.37.4 0 > ../md5/Linux-2.6.37.4
./signa -g ~/qemu/mem/mem-2.6.37.5 0 > ../md5/Linux-2.6.37.5
./signa -g ~/qemu/mem/mem-2.6.37.6 0 > ../md5/Linux-2.6.37.6
./signa -g ~/qemu/mem/mem-2.6.38.2 0 > ../md5/Linux-2.6.38.2
./signa -g ~/qemu/mem/mem-2.6.38.4 0 > ../md5/Linux-2.6.38.4
./signa -g ~/qemu/mem/mem-2.6.38.5 0 > ../md5/Linux-2.6.38.5
./signa -g ~/qemu/mem/mem-2.6.38.6 0 > ../md5/Linux-2.6.38.6
./signa -g ~/qemu/mem/mem-2.6.38.7 0 > ../md5/Linux-2.6.38.7
./signa -g ~/qemu/mem/mem-2.6.38.8 0 > ../md5/Linux-2.6.38.8
./signa -g ~/qemu/mem/mem-3.0.0 0 > ../md5/Linux-3.0.0
echo './signa -g ~/qemu/mem/mem-3.0.0 0 > ../md5/Linux-3.0.0'
./signa -g ~/qemu/mem/mem-3.0.4 0 > ../md5/Linux-3.0.4
echo "./signa -g ~/qemu/mem/mem-3.0.4 0 > ../md5/Linux-3.0.4"
./signa -g ~/qemu/mem/mem-solaris10-10 0 > ../md5/Solaris-10
echo "./signa -g ~/qemu/mem/mem-solaris10-10 0 > ../md5/Solaris-10"
