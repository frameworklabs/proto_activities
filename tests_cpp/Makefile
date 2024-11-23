run: tests tests17
	./tests
	./tests17

tests: tests.cpp ../include/proto_activities.h
	c++ --std c++14 -I ../include tests.cpp -o tests

tests17: tests.cpp ../include/proto_activities.h
	c++ --std c++17 -I ../include tests.cpp -o tests17

clean:
	rm tests
	rm tests17
