ps -ef | grep  myserverd| 
awk '{if (index($8,"myserverd")>0 && $3==1) print "kill -9",$2}'>stop_tmp.sh
ps -ef | grep  myserverd| 
awk '{if (index($8,"myserverd")>0 && $3!=1) print "kill -9",$2}'>>stop_tmp.sh
chmod 755 stop_tmp.sh
./stop_tmp.sh
rm stop_tmp.sh
