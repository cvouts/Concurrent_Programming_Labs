all: clean compile

clean: 
	@echo "Cleaning..."
	rm 3part1/primes -f
	rm 3part2/primenumbers -f
	rm 3part3/bridgecrossing -f
	rm 3part4/train -f

compile:
	@echo "Compiling..."
	gcc -Wall 3part1/primes.c -o 3part1/primes -lpthread
	gcc -Wall 3part2/bridgecrossing.c -o 3part2/bridgecrossing -lpthread
	gcc -Wall 3part3/train.c -o 3part3/train -lpthread
	gcc -Wall 3part4/bridgeCCR.c -o 3part4/bridgeCCR -lpthread -lrt