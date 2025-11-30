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

void ComplexPlane::updateRender()
{
	/*if (m_state == CALCULATING)
	{
		int width = m_pixel_size.x;
		int height = m_pixel_size.y;

		for (int y = 0; y < height; y++) //y = row
		{
			for (int x = 0; x < width; x++) //x = col
			{
				int index = x + y * width;
				m_vArray[index].position = { float(x), float(y) };

				Vector2f coord = mapPixelToCoords({ x, y });
				size_t count = countIterations(coord);

				Uint8 r, g, b;
				iterationsToRGB(count, r, g, b);

				m_vArray[index].color = { r, g, b };
			}
		}
		m_state = DISPLAYING;
	} */

	//Multithreaded version:
	if (m_state == CALCULATING)
	{
		int width = m_pixel_size.x;
		int height = m_pixel_size.y;

		int numThreads = thread::hardware_concurrency();
		if (numThreads <= 0) numThreads = 4; // fallback for safety

		vector<thread> threads;
		threads.reserve(numThreads);

		for (int t = 0; t < numThreads; t++)
		{
			threads.emplace_back([this, t, numThreads, width, height]()
				{
					for (int y = t; y < height; y += numThreads)
					{
						for (int x = 0; x < width; x++)
						{
							int index = x + y * width;

							m_vArray[index].position = { float(x), float(y) };

							Vector2f coord = mapPixelToCoords({ x, y });
							size_t count = countIterations(coord);

							Uint8 r, g, b;
							iterationsToRGB(count, r, g, b);

							m_vArray[index].color = { r, g, b };
						}
					}
				});
		}
		//combine all the threads together here
		for (auto& th : threads) { th.join(); }

		m_state = DISPLAYING;
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

	text.setString(ss.str());
}

size_t ComplexPlane::countIterations(Vector2f coord)
{
	float x = coord.x; //c.real
	float y = coord.y; //c.imaginary

	float zx = float(0);  //z.real starts at 0
	float zy = float(0);  //z.imag starts at 0

	size_t iteration = 0;
	const size_t MAX_ITERS = 64;

	while (iteration < MAX_ITERS)
	{
		float zx2 = zx * zx;
		float zy2 = zy * zy;

		if (zx2 + zy2 > float(4)) { break; } //if abs val of z > 2, break

		//z = z*z + c
		float tempzx = zx2 - zy2 + x; //new real part
		float tempzy = 2.0f * zx * zy + y; //new imaginary part

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
		//r = Uint8(9 * (1 - t) * t * t * t * 255);
		//g = Uint8(15 * (1 - t) * (1 - t) * t * t * 255);
		//b = Uint8(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
				//option2:
		//r = Uint8(255 * pow(t, 0.6f));       // pink
		//g = Uint8(200 * pow(t, 1.5f));       // aqua/teal
		//b = Uint8(255 * pow(1 - t, 2.5f));   // fading cotton glow
				//option 2:
		//float red = 255.0f * (0.9f * (1.0f - t) + 0.3f * t);
		//float green = 255.0f * (0.2f + 0.5f * t * t);
		//float blue = 255.0f * (0.4f + 0.6f * t);
		//float glow = 60.0f * (1.0f - fabs(0.5f - t) * 2.0f);
		//red = min(255.0f, red + glow);
		//green = min(255.0f, green + glow);
		//blue = min(255.0f, blue + glow);
		//r = Uint8(red);
		//g = Uint8(green);
		//b = Uint8(blue);
			//op 3:
		//r = Uint8(180 * t * t);             // purple-pink highlight
		//g = Uint8(20 * pow(1 - t, 3));     // dark shadow glow
		//b = Uint8(255 * pow(t, 0.4));       // main icy glow

		float baseR = 255.0f * pow(t, 0.35f);      // bright pinkish ice
		float baseG = 180.0f * pow(t, 2.0f);       // soft aqua midtones
		float baseB = 255.0f * pow(1 - t, 1.8f);   // deep icy blues

		// --- Frostfire wave effect (makes it look alive, NOT flat) ---
		float wave = 0.5f + 0.5f * sin(t * 12.0f + count * 0.05f);

		baseR *= (0.7f + 0.3f * wave);
		baseG *= (0.6f + 0.4f * wave);
		baseB *= (0.8f + 0.2f * wave);

		// --- Glow effect (makes edges shimmer) ---
		float glow = 90.0f * pow(1.0f - fabs(0.5f - t) * 2.0f, 2.0f);

		baseR = min(255.0f, baseR + glow * 0.9f);
		baseG = min(255.0f, baseG + glow * 0.5f);
		baseB = min(255.0f, baseB + glow * 1.2f);

		r = Uint8(baseR);
		g = Uint8(baseG);
		b = Uint8(baseB);


	}
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel)
{
	float real = ((float(mousePixel.x) - 0) / (float(m_pixel_size.x) - 0)) * (m_plane_size.x) + (m_plane_center.x - m_plane_size.x / 2.0);
	float imag = ((float(mousePixel.y) - float(m_pixel_size.y)) / (0 - float(m_pixel_size.y))) * (m_plane_size.y) + (m_plane_center.y - m_plane_size.y / 2.0);
	return { real, imag };
}



