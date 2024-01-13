all: tgt

tgt:
	cd src/ && $(MAKE)
	cp src/KNN .
	cp src/GraphSolve .
	rm src/KNN src/GraphSolve

clean:
	$(MAKE) -C src clean
	rm -f KNN GraphSolve

run:
	$(MAKE) -C src run