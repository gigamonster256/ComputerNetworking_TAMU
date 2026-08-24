.PHONY: all clean lint
all:

lint:
	./lint.sh

clean:
	$(MAKE) -C libtcp clean
	$(MAKE) -C libsbcp clean
	$(MAKE) -C libudp clean
	$(MAKE) -C libtftp clean
	$(MAKE) -C libhttp clean
	$(MAKE) -C libhttp/test clean
	$(MAKE) -C MP1 clean
	$(MAKE) -C MP2 clean
	$(MAKE) -C MP3 clean
	$(MAKE) -C MP4 clean
	rm result
