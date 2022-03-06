run: demo misc
	./demo 
	./misc

demo: demo.c ../include/proto_activities.h
	cc -I ../include demo.c -o demo

misc: misc.c ../include/proto_activities.h
	cc -I ../include misc.c -o misc
	
clean:
	rm demo
	rm misc
