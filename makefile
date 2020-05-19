CXX = g++
CPPFLAGS = -g -Wall -std=c++17 -I/usr/local/opt/freetype/include/freetype2
LIBS = -lglfw -lglew -framework OpenGL -framework Cocoa  -L/usr/local/opt/freetype/lib  -lfreetype
OBJ = main.o  imgui/imgui.a
DEPS = error.h model.h type.h string.h shader.h player.h camera.h collectible.h texture.h terrain.h math_utils.h obstacle.h font.h wall.h

build: main

%.o: %.cpp $(DEPS)
	$(CXX) -c $(CPPFLAGS) $< -o $@  

imgui/imgui.a:
	$(MAKE) -C imgui

main: $(OBJ)
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LIBS)

.PHONY:	clean
clean:
	rm -f $(OBJ)
