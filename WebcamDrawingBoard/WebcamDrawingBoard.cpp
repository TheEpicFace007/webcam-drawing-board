#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <thread>
#include <Windows.h>
#include "WebcamDrawingBoard.h"
//#include "pprint.h"

using namespace cv;
void nothing(int, void*) {}

int main(int argc, char** argv)
{
	VideoCapture capture;
	/* Set the capture setting */
	capture.set(CAP_PROP_FRAME_HEIGHT, 720);
	capture.set(CAP_PROP_FRAME_WIDTH, 1080);
	if (!capture.open(0))
	{
		MessageBoxW(NULL, L"Érreur dans le chargement de la capture vidéo\n"
			L"Votre caméra est pas plugger ou elle est utilisé dans une autre application.",
			NULL, 16);
		return 1;
	}

	/* Create the window */
	const String drawing_board = String("Tableau de note"); /* User view */
	cv::namedWindow(drawing_board, WINDOW_NORMAL);
	const String comptuter_drawing_board = String("Tableau de note[Vision de l'ordi]"); /* Computer view */
	cv::namedWindow(comptuter_drawing_board, WINDOW_AUTOSIZE);
	/* Create the trackbars */
	/* Lower range */
	cv::createTrackbar("L - H", comptuter_drawing_board, 0, 255);
	cv::createTrackbar("L - S", comptuter_drawing_board, 0, 255);
	cv::createTrackbar("L - V", comptuter_drawing_board, 0, 255);
	/* Upper range */
	cv::createTrackbar("U - H", comptuter_drawing_board, 0, 255);
	cv::createTrackbar("U - S", comptuter_drawing_board, 0, 255);
	cv::createTrackbar("U - V", comptuter_drawing_board, 0, 255);
	Mat frame;
	Mat mask_2; // the hsv image
	Mat mask_3;

	while (capture.read(frame))
	{
		cv::flip(frame, frame, 1);
		cv::flip(frame, mask_2, 1);
		// raise a message if the user want to quit 
		if (waitKey(10) == 27) // if escape is presed
		{
#ifndef _DEBUG
			const int SHOULD_QUIT = MessageBox(NULL, L"Est tu sure de vouloir quitter"
				L" le programme?", L"Quitté le programme?",
				MB_ICONQUESTION | MB_TOPMOST | MB_TASKMODAL | MB_YESNO);
			if (SHOULD_QUIT == IDYES) // if the answer is YES
#endif
				break;
#ifndef _DEBUG
			else
				continue;
#endif
		}
		// convert the bgr image to hsv
		cvtColor(frame, mask_2, COLOR_BGR2HSV);
		// set the capture setting before showing the capture
		// filter the image and get the binary mask which represent the target color
		cv::Mat lower_range;
		cv::Mat upper_range;
		cv::inRange(
			mask_2,
			Scalar(
				26,
				49,
				0
			),
			Scalar(
				133,
				255,
				255
			),
			lower_range
		);
		/*
		* converting the binary mask to 3 channel image, this is just so we can
		* stack it withh the others
		*/
		cv::cvtColor(
			lower_range,
			mask_3,
			COLOR_GRAY2BGR
		);

		// show the capture
		imshow(drawing_board, frame);
		imshow(comptuter_drawing_board, lower_range);
		imshow("frame 2", mask_3);
	}
	return 0;
}