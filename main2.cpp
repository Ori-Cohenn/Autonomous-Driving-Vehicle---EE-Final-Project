#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <gpiod.hpp>
#include <vector>

#define CameraWIDTH 400 //160  // 400 //640
#define CameraHeight 240 //120 // 240 //480

using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace gpiod; // line class changed to lineGPIO

Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, frameFinalDuplicate1;
Mat ROILane, ROILaneEnd;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd;

vector<int> histrogramLane;
vector<int> histrogramLaneEnd;
// Point2f Source[] = {Point2f(145, 250), Point2f(430, 250), Point2f(55, CameraHeight), Point2f(535, CameraHeight)}; // do not change
// Point2f Destination[] = {Point2f(160, 230), Point2f(410, 230), Point2f(160, 480), Point2f(410, 480)};
Point2f Source[] = {Point2f(40,135),Point2f(230,135),Point2f(10,230), Point2f(260,230)};
Point2f Destination[] = {Point2f(65,0),Point2f(200,0),Point2f(65,240), Point2f(200,240)};
// take the source and destination points and changed them accordingly for 160x120
// Point2f Source[] = {Point2f(20, 60), Point2f(115, 60), Point2f(0, 120), Point2f(145, 120)};
// Point2f Destination[] = {Point2f(20, 0), Point2f(115, 0), Point2f(20, 120), Point2f(115, 120)};

VideoCapture cap(0);

stringstream ss;

void setup()
{
    cap.set(CAP_PROP_FRAME_WIDTH, CameraWIDTH);   // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FRAME_HEIGHT, CameraHeight); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FPS, 100);
    cap.set(CAP_PROP_BRIGHTNESS, 15);
    cap.set(CAP_PROP_CONTRAST, 5);   // 50
    cap.set(CAP_PROP_SATURATION,100); // 50
    cap.set(CAP_PROP_GAIN, 20);      // 10
}



void captureFrames()
{
    if (cap.read(frame))
    {
        cvtColor(frame, frame, COLOR_BGR2RGB);
    }
}

void Perspective()
{
    line(frame, Source[0], Source[1], Scalar(0, 0, 255), 2);
    line(frame, Source[1], Source[3], Scalar(0, 0, 255), 2);
    line(frame, Source[3], Source[2], Scalar(0, 0, 255), 2);
    line(frame, Source[2], Source[0], Scalar(0, 0, 255), 2);
    line(frame, Destination[0], Destination[1], Scalar(0, 255, 0), 2);
    line(frame, Destination[1], Destination[3], Scalar(0, 255, 0), 2);
    line(frame, Destination[3], Destination[2], Scalar(0, 255, 0), 2);
    line(frame, Destination[2], Destination[0], Scalar(0, 255, 0), 2);
    Matrix = getPerspectiveTransform(Source, Destination);
    warpPerspective(frame, framePers, Matrix, Size(CameraWIDTH, CameraHeight));
}

void Threshold()
{
    cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
    inRange(frameGray, 250, 255, frameThresh);
    Canny(frameGray, frameEdge, 900, 900, 3, false);
    add(frameThresh, frameEdge, frameFinal);
    cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);  // used in histrogram function only
    cvtColor(frameFinal, frameFinalDuplicate1, COLOR_RGB2BGR); // used in histrogram function only
}

void Histrogram()
{
    histrogramLane.resize(CameraWIDTH);
    histrogramLane.clear();
    histrogramLaneEnd.resize(CameraWIDTH);
    histrogramLaneEnd.clear();
    for (int i = 0; i < CameraWIDTH; i++) // frame.size().width = 400
    {
        ROILane = frameFinalDuplicate(Rect(i, 140, 1, 100));
        // take the ROILane and adjust the values from 400x240 to 160x120
        // ROILane = frameFinalDuplicate(Rect(i, 60, 1, 60));
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0]));
        ROILaneEnd = frameFinalDuplicate1(Rect(i, 0, 1, CameraHeight));
        divide(255, ROILaneEnd, ROILaneEnd);
        histrogramLaneEnd.push_back((int)(sum(ROILaneEnd)[0]));
    }
    laneEnd = sum(histrogramLaneEnd)[0];
    cout << "Lane END = " << laneEnd << endl;
}

void LaneFinder()
{
    vector<int>::iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 140);
    // LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 60);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr);

    vector<int>::iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin()+140, histrogramLane.end()-20);
    // RightPtr = max_element(histrogramLane.begin() + 100, histrogramLane.end());
    RightLanePos = distance(histrogramLane.begin(), RightPtr);

    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255, 0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0, 255, 0), 2);
    // adjust the values from 400x240 to 160x120
    // line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 120), Scalar(0, 255, 0), 2);
    // line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 120), Scalar(0, 255, 0), 2);
}

void LaneCenter()
{
    laneCenter = (RightLanePos - LeftLanePos) / 2 + LeftLanePos;



    frameCenter = 133;
    // adjust the values from 400x240 to 160x120
    // frameCenter = 67;

    line(frameFinal, Point2f(laneCenter, 0), Point2f(laneCenter, 240), Scalar(0, 255, 0), 3);
    line(frameFinal, Point2f(frameCenter, 0), Point2f(frameCenter, 240), Scalar(255, 0, 0), 3);
    // adjust the values from 400x240 to 160x120
    // line(frameFinal, Point2f(laneCenter, 0), Point2f(laneCenter, 120), Scalar(0, 255, 0), 3);
    // line(frameFinal, Point2f(frameCenter, 0), Point2f(frameCenter, 120), Scalar(255, 0, 0), 3);

    Result = laneCenter - frameCenter;
}

int main()
{
    setup();
    const string gpio_chip = "gpiochip0";
    const unsigned int pins[] = {5, 6, 19, 26}; // 5 4 3 2 in the arduino, 5 6 19 26 in meaning GPIO 5,6,19,26 not the pins
    chip chip(gpio_chip);
    line_bulk lines(chip.get_lines({pins[0], pins[1], pins[2], pins[3]}));
    lines.request({{}, line_request::DIRECTION_OUTPUT});

    if (!cap.isOpened())
    {
        cerr << "Error opening webcam." << endl;
        return -1;
    }
    cout << "Webcam is opened" << endl;
    while (1)
    {
        int64 start = getTickCount();
        captureFrames();
        if (frame.empty())
        {
            cerr << "Error capturing the frame." << endl;
            break;
        }
        Perspective();
        Threshold();
        Histrogram();
        LaneFinder();
        LaneCenter();

        // if (laneEnd > 7000 && laneEnd<10000) // stop
        // {
        //     ss.str(" ");
        //     ss.clear();
        //     ss << " Lane End";
        //     putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
        //     lines.set_values({0, 1, 1, 1});
        //     // cout << "Lane End" << endl;
        // }
        if (Result == 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Forward";
            // putText(frame, ss.str(), Point2f(1, 50), 0, 1, Scalar(0, 0, 255), 2);
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 0, 0, 0});
            cout << "Forward" << endl;
        }

        else if (Result > 0 && Result < 10)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << "bMove Right";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 0, 0, 1}); // decimal = 1
            cout << "Right1" << endl;
        }

        else if (Result >= 10 && Result < 20)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << "bMove Right";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 0, 1, 0}); // decimal = 2
            cout << "Right2" << endl;
        }

        else if (Result >= 20)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << "bMove Right";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 0, 1, 1}); // decimal = 3
            cout << "Right3" << endl;
        }

        else if (Result < 0 && Result > -10)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Left";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 1, 0, 0}); // decimal =4
            cout << "Left1" << endl;
        }

        else if (Result <= -10 && Result > -20)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Left";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 1, 0, 1}); // decimal = 5
            cout << "Left2" << endl;
        }

        else if (Result <= -20)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Left";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 1, 1, 0}); // decimal = 6
            cout << "Left3" << endl;
        }

        namedWindow("orignal", WINDOW_KEEPRATIO);
        moveWindow("orignal", 0, 100);
        resizeWindow("orignal", 640, 480);
        imshow("orignal", frame);
        namedWindow("Perspective", WINDOW_KEEPRATIO);
        moveWindow("Perspective", 640, 100);
        resizeWindow("Perspective", 640, 480);
        imshow("Perspective", framePers);
        namedWindow("Final", WINDOW_KEEPRATIO);
        moveWindow("Final", 1280, 100);
        resizeWindow("Final", 640, 480);
        imshow("Final", frameFinal);
        waitKey(1);
        // if (waitKey(5) >= 0){
        //     break;
        // }
        // char c = (char)waitKey(1);
        // while (c == 'q' || c == 'Q' || c == 27)
        // {
        //     ss.str(" ");
        //     ss.clear();
        //     ss << " Lane End";
        //     putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
        //     lines.set_values({0, 1, 1, 1});
        //     cout << "Lane End" << endl;
        //     if ((char)waitKey(1) == 'c')
        //         break;
        // }
        double fps = getTickFrequency() / (getTickCount() - start);
        cout << "FPS : " << fps << endl;
    }
    return 0;
}