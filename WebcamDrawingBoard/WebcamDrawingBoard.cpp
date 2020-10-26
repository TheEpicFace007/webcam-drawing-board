
#include "WebcamDrawingBoard.h"
#include "Constant.h"
#include "Util.h"
#include "Keys.h"
#include <Windows.h>
#include <chrono>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#define null 0
//#include "pprint.h"

// enable this if we should compile what the program see
#define SHOW_COMPUTER_VIEW

using namespace cv;
using std::vector;
void nothing(int, void*) {}
/// <summary>
/// The function entrypoint
/// </summary>
/// <param name="argc">The argument count</param>
/// <param name="argv">The arguments string</param>
/// <returns>0</returns>
int main(int argc, char** argv)
{
    VideoCapture capture;
    /* Set the capture setting */
    capture.set(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1080);
    capture.set(VideoCaptureProperties::CAP_PROP_FPS, 60);
    capture.set(VideoCaptureProperties::CAP_PROP_BRIGHTNESS, 100);
    capture.set(VideoCaptureProperties::CAP_PROP_AUTOFOCUS, 1);
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
    vector<Rect> old_line_pt;

    bool is_drawing_enabled = true;
    while (capture.read(frame))
    {
        cv::flip(frame, frame, 1);
        cv::flip(frame, mask_2, 1);
        // raise a message if the user want to quit
        int SHOULD_QUIT;
        switch (waitKey(1))
        {
        case KeyboardKey::KEY_ESCAPE: // Escape is pressed

#ifndef _DEBUG
            SHOULD_QUIT = MessageBox(NULL, L"Est tu sure de vouloir quitter"
                L" le programme?",
                L"Quitté le programme?",
                MB_ICONQUESTION | MB_TOPMOST | MB_TASKMODAL | MB_YESNO);
            if (SHOULD_QUIT == IDYES) // if the answer is YES
                goto end;
            else
                break;
#endif
            goto end;
        case KeyboardKey::KEY_SPACE: // space is pressed
            old_line_pt.clear();
            break;
        case KeyboardKey::KEY_Z: // Z is pressed
            is_drawing_enabled = !is_drawing_enabled;
            if (is_drawing_enabled)
                std::cout << "Drawing enabled!\n";
            else
                std::cout << "Drawing disabled\n";
            break;
        }


        // convert the bgr image to hsv
        cv::cvtColor(frame, mask_2, COLOR_BGR2HSV);
        // set the capture setting before showing the capture
        // filter the image and get the binary mask which represent the target color
        cv::Mat lower_range;
        cv::Mat upper_range;
        cv::inRange(
            mask_2,
            Scalar(26, 49, 0),
            Scalar(133, 255, 255),
            lower_range);
        /*
        * converting the binary mask to 3 channel image, this is just so we can
        * stack it withh the others
        */
        cv::cvtColor(
            lower_range,
            mask_3,
            COLOR_GRAY2BGR);

        // perform morphological operation to get rid of the noise
        // erosion will remove the white noise while the dilation will expand it;
        cv::erode( // remove the white noise
            Scalar(26, 49, 0),
            Scalar(133, 255, 255),
            Mat::ones(Size(5, 5), 0),
            Point(-1, -1),
            100);
        cv::dilate( // enlarge the object
            Scalar(26, 49, 0),
            Scalar(133, 255, 255),
            Mat::ones(Size(5, 5), 0),
            cv::Point(-1, -1),
            2);
        // find the contour of the pen

        std::vector<std::vector<cv::Point>> contours;
        Mat contourOuput = mask_3.clone();
        cv::cvtColor(contourOuput, contourOuput, COLOR_BGR2GRAY);
        cv::findContours(contourOuput, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        // make sure there's a contour

        // contours[temp]
        int temp = -1;
        for (auto i = 0; i < contours.size(); i++)
        {
            if (contourArea(contours[i]) > temp && contourArea(contours[i]) >= constant::noise_threshold
                && contourArea(contours[i]) < 1000)

            {
                temp = i;
                std::cout << contourArea(contours[i]) << "\n";
                //std::cout << contourArea(contours[i]) << std::endl;
            }
        }

        if (temp != -1)
        {
            //  add a bit of ink
            Rect bounding_box_range = boundingRect(contours[temp]);
            old_line_pt.push_back(bounding_box_range);
            rectangle(frame, bounding_box_range, rgb2bgr(252, 211, 3), 2, LINE_4);
        }
        if (is_drawing_enabled)
            // add all ink to the frame
            for (size_t i = 0; i < old_line_pt.size(); i++)
            {
                line(
                    frame,
                    old_line_pt[i].tl(),
                    old_line_pt[i].br(),
                    Scalar(255, 0, 0),
                    1,
                    LineTypes::LINE_4);


            }
        // show the capture
        cv::imshow(drawing_board, frame);
        cv::imshow("frame 2", mask_3);
    }
end:
#ifndef _DEBUG
    std::cout << "Appuis sur n'importe quelle touche du clavier pour quitté le programme\n";
    std::cin.get();
#endif
    return 0;
}