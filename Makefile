CC=g++
LIBS=-lGL -lGLEW -lSDL2 -lm
NAME=mandel

make:
	rm -f $(NAME)
	$(CC) mandel.cpp Camera.cpp -o $(NAME) $(LIBS)
	rm -f *.o