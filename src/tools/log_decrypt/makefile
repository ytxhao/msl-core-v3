objects = DecryptProcessor.class


jar : $(objects)
	jar -cvfm spdlog-decrypt.jar META-INF/MANIFEST.MF -C build/classes/java/main .
	mkdir -p build/libs
	mv spdlog-decrypt.jar build/libs

DecryptProcessor.class :
	javac -sourcepath src/main/java -d build/classes/java/main src/main/java/com/stargroup/*.java

clean :
	rm -rf build