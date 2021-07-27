i686-w64-mingw32-gcc -c -o udpserver.o win-udpserver.c
i686-w64-mingw32-gcc -o udpserver.exe udpserver.o -lws2_32
gcc -o udpclient udpclient.c
gcc -o udpserver udpserver.c
gcc -o udpbftserver udpbftserver.c
