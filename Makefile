all: tgt

tgt:
	cd src/ && $(MAKE)
	cp src/KNN .
	rm src/KNN

clean:
	$(MAKE) -C src clean
	rm -f KNN GraphSolve

run:
	$(MAKE) -C src run