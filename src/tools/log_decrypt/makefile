
SRC_DIR = src/main/java
CLASSES_DIR = build/classes/java/main
MKFILE_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

objects = $(notdir $(shell find ${SRC_DIR} -type f -iname '*.java'))

ifeq ($(PREFIX),) 
    PREFIX := $(MKFILE_DIR)
endif 


# 隐晦规则,隐含规则,隐式规则
# .SUFFIXES: .java .class
# .class:
# 	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $< | sed 's/ /\\ /g')

spdlog-decrypt.jar : $(objects)
	@jar -cvfm $@ META-INF/MANIFEST.MF -C ${CLASSES_DIR} .
	@mkdir -p build/libs
	@mv $@ build/libs

%.java:
	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $@ | sed 's/ /\\ /g')

install :
	install -d $(PREFIX)/lib
	install -m 644 build/libs/spdlog-decrypt.jar $(PREFIX)/lib

uninstall:
	rm -f $(PREFIX)/lib/spdlog-decrypt.jar

clean :
	rm -rf build