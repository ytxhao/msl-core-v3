# objects = DecryptProcessor.java
SRC_DIR = src/main/java
CLASSES_DIR = build/classes/java/main

# objects = $(shell find ${SRC_DIR} -type f -iname '*.java')
# src = $(shell find ${SRC_DIR} -type f -iname '*.java')
# objects = $(notdir $(shell find ${SRC_DIR} -type f -iname '*.java' | sed 's/.java/.class/g'))
objects = $(notdir $(shell find ${SRC_DIR} -type f -iname '*.java'))
# objects = $(source:.java=.class)

# objects = $(wildcard *.java)
# MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
# MKFILE_DIR := $(dir $(MKFILE_PATH))
MKFILE_DIR = $(lastword $(MAKEFILE_LIST))
# MKFILE_PATH2 = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
$(warning =======${src})
$(warning =======${MKFILE_DIR})
$(warning =======objects:${objects})
# $(warning =======${MKFILE_PATH2})

MKFILE_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))



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

# %.class: %.java
# 	echo $<
# 	echo $@
#	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $< | sed 's/ /\\ /g')

# %.class:$(shell find ${SRC_DIR} -type f -iname %.java)
%.java:
	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $@ | sed 's/ /\\ /g')
#	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $<
	echo $@
	echo $<
#	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $< | sed 's/ /\\ /g')

# DecryptProcessor.class:
# 	echo $@
# 	echo $<
#	@javac -sourcepath ${SRC_DIR} -d ${CLASSES_DIR} $(shell find ${SRC_DIR} -type f -iname $< | sed 's/ /\\ /g')

install :
	install -d $(PREFIX)/lib
	install -m 644 build/libs/spdlog-decrypt.jar $(PREFIX)/lib

uninstall:
	rm -f $(PREFIX)/lib/spdlog-decrypt.jar

clean :
	rm -rf build