all: sv_node

sv_node.o: sv_node.cc
	g++ -c sv_node.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
message_header.o: message_header.cc message_header.h
	g++ -c message_header.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
hello.o: hello.cc hello.h
	g++ -c hello.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
file.o: file.cc file.h
	g++ -c file.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
socket_list.o: socket_list.cc socket_list.h
	g++ -c socket_list.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
timer.o: timer.cc timer.h message_header.h message_header.cc
	g++ -c timer.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g
linklist_beacons.o: linklist_beacons.cc linklist_beacons.h
	g++ -c linklist_beacons.cc -g -Wall
message_cache.o: message_cache.cc message_cache.h
	g++ -c message_cache.cc -g -Wall
neighbour_list.o: neighbour_list.cc neighbour_list.h
	g++ -c neighbour_list.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g	
commandLine.o: commandLine.cc
	g++ -c commandLine.cc -Wno-deprecated -Wall -I/home/scf-22/csci551b/openssl/include -g	


sv_node: sv_node.o commandLine.o neighbour_list.o message_cache.o message_header.o hello.o file.o socket_list.o timer.o linklist_beacons.o
	g++ sv_node.o commandLine.o neighbour_list.o message_header.o message_cache.o hello.o file.o socket_list.o timer.o linklist_beacons.o -o sv_node -lnsl -lresolv -lsocket -lpthread -L/home/scf-22/csci551b/openssl/lib -lcrypto

clean: 
	rm -rf *.o sv_node beacons_stored

