run: demo
	./demo 

demo: demo.cpp ../include/proto_activities.h
	c++ --std c++17 -I ../include demo.cpp -o demo
	
clean:
	rm demo
