
CXX = c++
CPPFLAGS = -Wall
OBJS = imgui.o imgui_demo.o imgui_draw.o imgui_widgets.o

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CPPFLAGS)

imgui.a: $(OBJS)
	$(AR) rvs $@ $^
