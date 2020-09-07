all: clean compile

clean: 
	@echo "Cleaning..."
	rm 2part1/mysem -f
	rm 2part2/primenumbers -f
	rm 2part3/bridgecrossing -f
	rm 2part4/train -f

compile:
	@echo "Compiling..."
	gcc -Wall 2part1/mysem.c -o 2part1/mysem -c -lpthread
	gcc -Wall 2part2/primenumbers.c -o 2part2/primenumbers -lpthread
	gcc -Wall 2part3/bridgecrossing.c -o 2part3/bridgecrossing -lpthread
	gcc -Wall 2part4/train.c -o 2part4/train -lpthread

