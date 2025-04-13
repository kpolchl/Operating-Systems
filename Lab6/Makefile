CXX=gcc
CFLAGS=-Wall -std=c17
calculateIntegral:
	$(CXX) $(CFLAGS) -o calculateIntegral.out calculateIntegral.c
calculation:
	$(CXX) $(CFLAGS) -o calculation.out  calculation.c
givedata:
	$(CXX) $(CFLAGS) -o givedata.out givedata.c

all: calculateIntegral calculation givedata
clean:
	rm -f calculateIntegral.out givedata.out calculation.out 
.PHONY: clean all