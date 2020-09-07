all: clean compile

clean: 
	@echo "Cleaning..."
	rm mycoroutines -f
	rm mythreads -f
	rm primes -f
	
compile:
	@echo "Compiling..."
	gcc -Wall mycoroutines.c -o mycoroutines -c
	gcc -Wall mythreads.c -o mythreads -c
	gcc -Wall primes.c mythreads.c -o primes
	
	
