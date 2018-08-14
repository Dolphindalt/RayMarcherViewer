CC=g++
LIBS=-lGL -lGLEW -lSDL2 -lm
NAME=mandel
FLAGS=-I./imgui -I./imgui/examples
SOURCES= mandel.cpp Camera.cpp
SOURCES+= ./imgui/imgui.cpp ./imgui_draw.cpp
SOURCES+= ./imgui/examples/imgui_impl_sdl.cpp ./imgui/examples/imgui_impl_opengl3.cpp
OBJS= $(addsuffix .o, $(basename $(notdir $(SOURCES))))

%.o:%.cpp
	$(CC) $(FLAGS) -c -o $@ $< $(LIBS)

%.o:./imgui/%.cpp
	$(CC) $(FLAGS) -c -o $@ $< $(LIBS)

%.o:./imgui/examples/%.cpp
	$(CC) $(FLAGS) -c -o $@ $< $(LIBS)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(FLAGS) $(LIBS)

clean:
	rm -f $(NAME) $(OBJS)