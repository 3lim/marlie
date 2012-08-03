/*  Example: How to use the SimpleImage Class
	Author: Ismail Demir
*/

#include <iostream>
#include "SimpleImage.h"


int main(int argc, char * argv[]) {

	// Create an empty image of size 640 x 480
	SimpleImage my_first_image(640, 480);

	// Fill the image with a color gradient
	 for(int y=0; y<480; ++y)
		 for(int x=0; x<640; ++x)
			 // Set RGB Values at every pixel (x,y)
			 my_first_image.setPixel(x,y, (BYTE)(255.0f * x / 640.0f), (BYTE)(255.0f * y / 480.0f), 0);

	 // Save my_first_image into "test.png"
	 // Note: Only PNG Format is supported
	 my_first_image.save("test.png");

	 // Load "test.png" into the image test
	 SimpleImage test("test.png");

	 // Read the RGB values at pixel (100,200)
	 BYTE r,g,b;
	 test.getPixel(100,200,r,g,b);

	 // Show RGB values on the screen
	 printf("R: %d, G: %d, B: %d\n", r, g, b);

	 system("pause");


}