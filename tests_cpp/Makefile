run: tests
	./tests

tests: tests.cpp ../include/proto_activities.h
	c++ -I ../include tests.cpp -o tests
	
clean:
	rm tests
