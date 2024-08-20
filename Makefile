building_demo: building_demo.c rotation_demo.o
	gcc -g -Wall -Wextra -o0 building_demo.c rotation_demo.o -lcurses -lm -o building_demo

rotation_demo.o: rotation_demo.c rotation_demo.h
	gcc -g -o0 -Wall -Wextra -o0 -c rotation_demo.c -o rotation_demo.o

clean:
	rm -f *.o building_demo
