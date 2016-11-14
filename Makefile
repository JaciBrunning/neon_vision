# This will need some changes depending on where you saved OpenCV to.

OBJECTS = main.o
ASM = mem.o
CFLAGS = -mfpu=neon -mcpu=cortex-a9 -I../OpenCV/include -L../OpenCV/libs -lopencv_imgproc -lopencv_core -lrt -lpthread -lm -ldl -lz -ltbb
COMP_PREFIX = arm-frc-linux-gnueabi-

all: compile

compile:
	-mkdir -p build/bin
	$(COMP_PREFIX)g++ $(CFLAGS) -o build/bin/buildfile $(ASM:%.o=src/asm/%.S) $(OBJECTS:%.o=src/c/%.cpp)

deploy: all
	scp build/bin/buildfile lvuser@172.22.11.2:/home/lvuser/neon_vision

clean:
	rm -r build/