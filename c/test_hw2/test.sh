#!/bin/sh
name=localhost
./delayedclient_new.o $name 8000 test1.txt > out1
./delayedclient_new.o $name 8000 test2.txt > out2
./delayedclient_new.o $name 8000 test3.txt > out3
./delayedclient_new.o $name 8000 test4.txt > out4
