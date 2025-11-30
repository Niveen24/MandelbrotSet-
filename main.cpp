#include <iostream>
#include <SFML/Graphics.hpp>
#include "ComplexPlane.h"

using namespace std;
using namespace sf;

int main() {

	int width = VideoMode::getDesktopMode().width;
	int height = VideoMode::getDesktopMode().height;
	RenderWindow window(VideoMode(width, height), "Mandelbrot");	
	ComplexPlane complexPlane(width, height);

	Font font;
	font.loadFromFile("fonts/Nabla.ttf");
	Text infoText;
	infoText.setFont(font);
	infoText.setCharacterSize(20);
	infoText.setFillColor(Color(139,86,245)); //set custom color
	infoText.setPosition(float(10), float(10));

	while (window.isOpen())
	{
		//Input
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
			{
				window.close();
			}
			else if (event.type == Event::MouseButtonPressed) //if mouse clicked
			{
				if (event.mouseButton.button == Mouse::Right)	//right click zooms out
				{
					complexPlane.zoomOut();
					complexPlane.setCenter({ event.mouseButton.x, event.mouseButton.y });
				}
				else if (event.mouseButton.button == Mouse::Left) //left click zooms in
				{
					complexPlane.zoomIn();
					complexPlane.setCenter({ event.mouseButton.x, event.mouseButton.y });
				}
			}
			else if (event.type == Event::MouseMoved)	//if mouse moved, update location (so coords change and text updates)
			{
				complexPlane.setMouseLocation({ event.mouseMove.x, event.mouseMove.y }); 
			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}

		//Update
		complexPlane.updateRender();
		complexPlane.loadText(infoText);

		//Draw
		window.clear();
		window.draw(complexPlane);
		window.draw(infoText);
		window.display();
	}

	return 0;
}
