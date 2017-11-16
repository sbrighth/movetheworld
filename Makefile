DIRS = tbase tnet tdm texec tdebug

all: 
	for dir in $(DIRS); do \
		$(MAKE) all -C $$dir; \
	done

clean:
	for dir in $(DIRS); do \
		$(MAKE) clean -C $$dir; \
	done

copy:
	mkdir -p /tmp/exicon
	cp -rf output/* /tmp/exicon/

.PONY: all clean copy
