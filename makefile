.PHONY: all clean
all:

clean:
	$(MAKE) -C libtcp clean
	$(MAKE) -C libsbcp clean
	$(MAKE) -C libudp clean
	$(MAKE) -C libtftp clean
	$(MAKE) -C libhttp clean
	$(MAKE) -C libhttp/test clean
	$(MAKE) -C MP1/src clean
	$(MAKE) -C MP2/src clean
	$(MAKE) -C MP3/src clean
	$(MAKE) -C MP4/src clean
