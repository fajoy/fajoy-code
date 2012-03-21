#!/bin/sh


echo '$!'=$!
sleep 10&
echo '$$'=$$
echo '$!'=$!
echo '$-'=$-
echo '$#'=$#
echo '$@'=$@
echo '$*'=$*
echo '$0'=$0
echo '$1'=$1
echo '$2'=$2


echo '${var}='$var
echo '${var:=aaa,bbb,ccc}='${var:=aaa,bbb,ccc sss}
echo '${var}='${var}
echo '${var%b*}='${var%b*}
echo '${var%%b*}='${var%%b*}
echo '${var#*b}='${var#aaa,bbb}
echo '${var##*b}='${var##*b}
echo '${var%b*#*b}='${var%b*}
echo '${#var}='${#var}
echo '${var#var}='${var#var}
echo '${var:3}='${var:3}
echo '${var:3:5}='${var:3:5}
echo '${var/,*,/,ddd,}='${var/,*,/,ddd,}

echo 'a{a,b,c}='a{a,b,c}c

