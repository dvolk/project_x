all:
	g++ -g --std=c++11 -Wall main.cpp -o main `pkg-config --cflags --libs allegro-5.0 allegro_primitives-5.0 allegro_color-5.0`

clean:
	rm main
