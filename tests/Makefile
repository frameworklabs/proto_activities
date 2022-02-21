run: tests
	./tests

tests: tests.c ../include/proto_activities.h
	cc -I ../include tests.c -o tests
	
clean:
	rm tests
