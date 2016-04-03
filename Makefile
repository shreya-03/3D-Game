all:  sample2D

sample2D: main.cpp
	g++ -o sample2D main.cpp -lGL -lGLU -lGLEW -lglut 
clean:
	rm sample2D sample3D

