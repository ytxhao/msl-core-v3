objects = DecryptProcessor.java

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))

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
	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} ${SRC_DIR}/com/stargroup/$@

install :
	install -d $(PREFIX)/lib
	install -m 644 build/libs/spdlog-decrypt.jar $(PREFIX)/lib

uninstall:
	rm -f $(PREFIX)/lib/spdlog-decrypt.jar

clean :
	rm -rf build