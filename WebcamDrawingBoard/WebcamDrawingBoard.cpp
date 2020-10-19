
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <thread>
#include <Windows.h>
#include "WebcamDrawingBoard.h"
#include "Constant.h"
//#include "pprint.h"

using namespace cv;
void nothing(int, void*) {}

int main(int argc, char** argv)
{
	VideoCapture capture;
	/* Set the capture setting */
	capture.set(CAP_PROP_FRAME_HEIGHT, 720);
	capture.set(CAP_PROP_FRAME_WIDTH, 1080);
	capture.set(CAP_PROP_FPS, 60);
	//capture.set(CAP_PROP_SATURATION, 20);
	if (!capture.open(0))
	{
		MessageBoxW(NULL, L"Érreur dans le chargement de la capture vidéo\n"
			L"Votre caméra est pas plugger ou elle est utilisé dans une autre application.",
			NULL, 16);
		return 1;
	}

	// this threshold is used to filter the noise to understand what a contout is
	

	/* Create the window */
	const String drawing_board = String("Tableau de note"); /* User view */
	cv::namedWindow(drawing_board, WINDOW_NORMAL);
	Mat frame;
	Mat mask_2; // the hsv image
	Mat mask_3;
	for (int i = 0; i < 2; i++)
		std::cout << std::string("---------------------------------------------------------\n");
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
			Scalar(26, 49, 0),
			Scalar(133, 255, 255),
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

		// perform morphological operation to get rid of the noise
		// erosion will remove the white noise while the dilation will expand it;
		cv::erode( // remove the white noise
			Scalar(26, 49, 0),
			Scalar(133, 255, 255),
			Mat::ones(Size(5, 5), 0),
			Point(-1, -1),
			1
		);
		cv::dilate( // enlarge the object
			Scalar(26, 49, 0),
			Scalar(133, 255, 255),
			Mat::ones(Size(5, 5), 0),
			cv::Point(-1, -1),
			10
		);
		// find the contour of the pen

		std::vector<std::vector<cv::Point> > contours;
		Mat contourOuput = mask_3.clone();
		cvtColor(contourOuput, contourOuput, COLOR_BGR2GRAY);
		cv::findContours(contourOuput, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		// make sure there's a contour

		// contours[temp]
		int temp = -1;
		for (auto i = 0; i < contours.size(); i++)
		{
			if (   contourArea(contours[i]) > temp 
				&& contourArea(contours[i]) >= constant::noise_threshold
				&& contourArea(contours[i]) < 1500 /*<==check if the contour is my leg */ )
			{
				temp = i;
				//std::cout << contourArea(contours[i]) << std::endl;

			}
		}
		if (temp != -1)
		{
			Rect bounding_box_range = boundingRect(contours[temp]);
			rectangle(frame, bounding_box_range, Scalar(30, 135, 30), 2, LINE_4);
		}
		// show the capture
		imshow(drawing_board, frame);
		imshow("frame 2", mask_3);
	}
	return 0;
}