objects = DecryptProcessor.java

# MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
# MKFILE_DIR := $(dir $(MKFILE_PATH))

# MKFILE_PATH2 = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
# $(warning =======${MKFILE_PATH})
# $(warning =======${MKFILE_PATH2})

MKFILE_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC_DIR = src/main/java
CLASSES_DIR = build/classes/java/main

ifeq ($(PREFIX),) 
    PREFIX := $(MKFILE_DIR)
endif 

spdlog-decrypt.jar : $(objects)
	@jar -cvfm $@ META-INF/MANIFEST.MF -C ${CLASSES_DIR} .
	@mkdir -p build/libs
	@mv $@ build/libs

DecryptProcessor.java :
	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $@ | sed 's/ /\\ /g')

install :
	install -d $(PREFIX)/lib
	install -m 644 build/libs/spdlog-decrypt.jar $(PREFIX)/lib

uninstall:
	rm -f $(PREFIX)/lib/spdlog-decrypt.jar

clean :
	rm -rf build