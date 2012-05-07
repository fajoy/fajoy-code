CC = gcc
FLAG =   
JDK_HOME = /opt/sun-jdk-1.6.0.29
all:
	$(CC) Hello.c -o Hello.o -fPIC -c -I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/
	$(CC) -shared -o libhello.so Hello.o
head:
	javah -jni Hello
