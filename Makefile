.PHONY: KNN tests

all: KNN

KNN:
	cd src/ && $(MAKE)
	cp src/KNN .
	cp src/GraphSolve .
	rm src/KNN src/GraphSolve

tests:
	cd tests/ && $(MAKE)

clean:
	$(MAKE) -C src clean
	rm -f KNN GraphSolve

run:
	$(MAKE) -C src run