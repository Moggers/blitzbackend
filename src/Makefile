LIBS=$(shell mysql_config --libs) -lconfig -lm
all: clean main.o table.o match.o matchhandler.o matchinstance.o settings.o util.o matchwatcher.o
	clang++ main.o table.o match.o matchhandler.o matchinstance.o settings.o util.o matchwatcher.o $(LIBS) -o ../bin/a.out -g --std=c++11 

clean:
	rm -rf *.o ../bin/a.out

main.o: main.cpp
	clang++ -c -I/usr/include/mysql main.cpp -g --std=c++11

table.o: table.cpp
	clang++ -c -I/usr/include/mysql table.cpp -g --std=c++11

match.o: match.cpp
	clang++ -c -I/usr/include/mysql match.cpp -g --std=c++11

matchhandler.o: matchhandler.cpp
	clang++ -c -I/usr/include/mysql matchhandler.cpp -g --std=c++11

matchinstance.o: matchinstance.cpp
	clang++ -c -I/usr/include/mysql matchinstance.cpp -g --std=c++11

settings.o: settings.cpp
	clang++ -c -I/usr/include/mysql settings.cpp -g --std=c++11

util.o: util.cpp
	clang++ -c -I/usr/include/mysql util.cpp -g --std=c++11

matchwatcher.o: matchwatcher.cpp
	clang++ -c -I/usr/include/mysql matchwatcher.cpp -g --std=c++11
