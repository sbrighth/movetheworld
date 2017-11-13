DIRS = tbase tnet tdm texec test

all: 
	for dir in $(DIRS); do \
		$(MAKE) all -C $$dir; \
	done

clean:
	for dir in $(DIRS); do \
		$(MAKE) clean -C $$dir; \
	done

copy:
	cp -rf output/* /exicon/

.PONY: all clean copy
