all: clean compile

clean: 
	@echo "Cleaning..."
	rm coroutinecheck -f
	rm mycoroutines -f
	rm output.txt -f

compile:
	@echo "Compiling..."
	gcc -Wall mycoroutines.c -o mycoroutines -c
	gcc -Wall coroutinecheck.c -o coroutinecheck


	
