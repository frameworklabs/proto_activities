run: tests
	./tests

tests: tests.cpp ../include/proto_activities.h
	c++ --std c++14 -I ../include tests.cpp -o tests
	
clean:
	rm tests
