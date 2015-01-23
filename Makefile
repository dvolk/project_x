all:
	g++ -g -Wall -pedantic --std=c++11 main.cpp -o main `pkg-config --cflags --libs allegro-5.0 allegro_primitives-5.0 allegro_color-5.0 allegro_image-5.0 allegro_font-5.0 sigc++-2.0`

clean:
	rm main
