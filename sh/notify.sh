#!/bin/sh
DB_DEFAULTS_FILE="`dirname $0`/mysql.ini"
DB_Query="mysql --defaults-file=${DB_DEFAULTS_FILE} -e"
DB_QueryPrefix="SET NAMES 'utf8';"

db_query()
{
   $DB_Query "$DB_QueryPrefix $1"
}


mail_notify()
{
   to=$1;subject=$2;content=$3;
        echo \
"From: root@localhost
To: $to
Subject: $subject
Content-Type: text/plain; charset=\"utf8\"
Content-Transfer-Encoding: 8bit

$content
   " |`sendmail -t`
}

show(){
while read line ;do 
echo "${line}" 
done;

}

sql='
set @yday=current_date() - interval 1 day;
set @tday=current_date();
Select  * from `tb_test` where `date` between @yday and @tday'
data=`db_query "$sql"`
#echo "${data}" | show

mail_notify "wuminfajoy@gmail.com" "[test]test" "${data}"
