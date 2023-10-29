all: tgt

tgt:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

run:
	$(MAKE) -C src run