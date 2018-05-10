CC=g++
MODE=-m64 -std=c++11
all: myserverd 
myserverd: daemon.o comm_thread.o commun.o config.o \
dispatch_req.o loginfo.o oradb.o packet.o \
SignInSys_db.o SignInSys_Function.o

	${CC} ${MODE} -o myserverd daemon.o \
comm_thread.o config.o commun.o dispatch_req.o \
loginfo.o oradb.o packet.o \
SignInSys_db.o SignInSys_Function.o \
-L/usr/lib64/mysql -lmysqlclient -lpthread  

daemon.o: daemon.cpp
	${CC} ${MODE} -c -pthread -Wall daemon.cpp
comm_thread.o: comm_thread.cpp
	${CC} ${MODE} -c -pthread -Wall comm_thread.cpp
commun.o: commun.cpp
	${CC} ${MODE} -c -pthread -Wall commun.cpp
config.o: config.cpp
	${CC} ${MODE} -c -pthread -Wall config.cpp
dispatch_req.o: dispatch_req.cpp
	${CC} ${MODE} -c -pthread -Wall dispatch_req.cpp
loginfo.o: loginfo.cpp
	${CC} ${MODE} -c -pthread -Wall loginfo.cpp
oradb.o: oradb.cpp
	${CC} ${MODE} -c -pthread -Wall oradb.cpp
packet.o: packet.cpp
	${CC} ${MODE} -c -pthread -Wall packet.cpp
SignInSys_db.o: SignInSys_db.cpp
	${CC} ${MODE} -c -pthread -Wall SignInSys_db.cpp
SignInSys_Function.o: SignInSys_Function.cpp
	${CC} ${MODE} -c -pthread -Wall SignInSys_Function.cpp
	
clean:
	rm *.o
