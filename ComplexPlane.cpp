#include "ComplexPlane.h"

ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight)
{
	m_pixel_size = { pixelWidth, pixelHeight }; //sets pixel side to the parameter values (in 2d vector)
	m_aspectRatio = float(pixelHeight) / float(pixelWidth); //calculates the aspect ratio of monitor as float
	m_plane_center = { 0, 0 }; //sets plane center to origin
	m_plane_size = { BASE_WIDTH, BASE_HEIGHT * m_aspectRatio }; //sets plane size to base width and height (height adjusted for aspect ratio)
	m_zoomCount = 0; //initial zoom is 0
	m_state = State::CALCULATING; //initial state is calculating
	//initialize vertex array
	m_vArray.setPrimitiveType(Points);
	m_vArray.resize(pixelWidth * pixelHeight);
}

void ComplexPlane::draw(RenderTarget& target, RenderStates states) const
{
	target.draw(m_vArray);
}

void complexPlane::updateRender()
{
	if (m_state == State::CALCULATING)
	{
		for (int i = 0; i < pixelWidth; i++)	//i for y (height)
		{
			for (int j = 0; j < pixelHeight; j++)	//j for x (width)
			{
				vArray[j + i * pixelWidth].position = { (float)j, (float)i }; //sets pos of array element that == screen coords j,i
				Vector2f coord = mapPixelToCoords({ j, i }); //maps pixels to complex plane coords
				size_t iterations = countIterations(coord);	//counts how many iterations for that coord
				Uint8 r, g, b;	//just declaring rgb values
				iterationsToRGB(iterations, r, g, b); //maps the iterations to rgb vals
				vArray[j + i * pixelWidth].color = { r, g, b };	//sets color of the elements that == screen coords j and i
			}
			m_state = State::DISPLAYING;
		}
	}
}

void ComplexPlane::zoomIn()
{
	m_zoomCount++;
	float newWidth = BASE_WIDTH * pow(BASE_ZOOM, m_zoomCount); //calculates new zoomed in width
	float newHeight = BASE_HEIGHT * m_aspectRatio * pow(BASE_ZOOM, m_zoomCount); //calculates new zoomed in height
	m_plane_size = { newWidth, newHeight }; //sets new plane size
	m_state = State::CALCULATING;
}

void ComplexPlane::zoomOut() //same as zoomIn but decrements zoomCount instead of incrementing
{
	m_zoomCount--;
	float newWidth = BASE_WIDTH * pow(BASE_ZOOM, m_zoomCount);
	float newHeight = BASE_HEIGHT * m_aspectRatio * pow(BASE_ZOOM, m_zoomCount);
	m_plane_size = { newWidth, newHeight };
	m_state = State::CALCULATING;
}

void ComplexPlane::setCenter(Vector2i mousePixel)
{
	m_plane_center = mapPixelToCoords(mousePixel);
	m_state = State::CALCULATING;
}

void ComplexPlane::setMouseLocation(Vector2i mousPixel)
{
	m_mouseLocation = mapPixelToCoords(mousPixel);
}

void ComplexPlane::loadText(Text& text)
{
	stringstream ss;
	ss << "Mandelbrot Set" << endl;
	ss << "Center: (" << m_plane_center.x << ", " << m_plane_center.y << ")" << endl;
	ss << "Cursor: (" << m_mouseLocation.x << ", " << m_mouseLocation.y << ")" << endl;
	ss << "Left-click to Zoom in" << endl;
	ss << "Right-click to Zoom out" << endl;
}

size_t ComplexPlane::countIterations(Vector2f coord)
{
	//TODO: count number of iterations of the set fore the given coordinate
	float x = float(0); //x of z : real
	float y = float(0);	//y of z : imaginary 
	float zx = coord.x; //x of c : real
	float zy = coord.y; //y of c : imaginary

	size_t iteration = 0;
	const size_t MAX_ITERS = 64;

	while (iteration < MAX_ITERS)
	{
		float zx2 = zx * zx;
		float zy2 = zy * zy;

		if ((zx2 + zy2) > float(4)) //2^2 = 4, check to see if escape radius reached
		{
			break;
		}

		float tempzx = zx2 - zy2 + x;
		float tempzy = float(2) * zx * zy + y;
		zx = tempzx;
		zy = tempzy;

		iteration++;
	}

	return iteration;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b)
{
	if (count == MAX_ITER) //set to black if max iters reached
	{
		r = 0;
		g = 0;
		b = 0;
	}
	else
	{
		float t = float(count) / float(MAX_ITER);
		r = Uint8(9 * (1 - t) * t * t * t * 255);
		g = Uint8(15 * (1 - t) * (1 - t) * t * t * 255);
		b = Uint8(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
	}
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel)
{
	float real = ((float(mousePixel.x) - 0) / (float(m_pixel_size.x) - 0)) * (m_plane_size.x) + (m_plane_center.x - m_plane_size.x / 2.0);
	float imag = ((float(mousePixel.y) - float(m_pixel_size.y)) / (0 - float(m_pixel_size.y))) * (m_plane_size.y) + (m_plane_center.y - m_plane_size.y / 2.0);
	return { real, imag };
}