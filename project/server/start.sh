ps -ef | grep  myserverd| 
awk '{if (index($8,"myserverd")>0 && $3==1) print "kill -9",$2}'>start_tmp.sh
ps -ef | grep  myserverd| 
awk '{if (index($8,"myserverd")>0 && $3!=1) print "kill -9",$2}'>>start_tmp.sh
#echo "./myserverd" >> start_tmp.sh
echo "./myserverd" >> start_tmp.sh
chmod 755 start_tmp.sh
./start_tmp.sh
rm start_tmp.sh
