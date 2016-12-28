simulator: Apex_main.o 
	g++ -g -o ../bin/simulator Apex_main.o 

main.o : main.cpp 
	g++ -g -c Apex_main.cpp
clear:
	rm -f Apex_main.o 
