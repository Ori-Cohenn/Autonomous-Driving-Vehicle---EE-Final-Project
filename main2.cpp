#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <gpiod.hpp>

#define CameraWIDTH  640
#define CameraHeight 480

using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace gpiod; // line class changed to lineGPIO

Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, frameFinalDuplicate1;
Mat ROILane, ROILaneEnd;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd;

vector<int> histrogramLane;
vector<int> histrogramLaneEnd;
Point2f Source[] = {Point2f(145,250), Point2f(430,250), Point2f(55,CameraHeight), Point2f(535, CameraHeight)}; //do not change
Point2f Destination[] = {Point2f(160, 230), Point2f(410, 230), Point2f(160, 480), Point2f(410, 480)};
// Point2f Source[] = {Point2f(50,145),Point2f(255,145),Point2f(15,195), Point2f(290,195)};
// Point2f Destination[] = {Point2f(75,0),Point2f(220,0),Point2f(75,240), Point2f(220,240)};


VideoCapture cap(0);

stringstream ss;

void setup()
{
    cap.set(CAP_PROP_FRAME_WIDTH, CameraWIDTH);  // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FRAME_HEIGHT, CameraHeight); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FPS,0);          
    cap.set(CAP_PROP_BRIGHTNESS, 0);
    cap.set(CAP_PROP_CONTRAST, 80);
    cap.set(CAP_PROP_SATURATION, 50);
    cap.set(CAP_PROP_GAIN, 10);
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
    line(frame, Destination[0], Destination[1], Scalar(0, 255,0), 2);
    line(frame, Destination[1], Destination[3], Scalar(0, 255, 0), 2);
    line(frame, Destination[3], Destination[2], Scalar(0, 255, 0), 2);
    line(frame, Destination[2], Destination[0], Scalar(0, 255, 0), 2);
    Matrix = getPerspectiveTransform(Source, Destination);
    warpPerspective(frame, framePers, Matrix, Size(CameraWIDTH, CameraHeight));
}

void Threshold()
{
    cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
    inRange(frameGray, 240, 255, frameThresh);
    Canny(frameGray, frameEdge, 900, 900, 3, false);
    add(frameThresh, frameEdge, frameFinal);
    cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR); // used in histrogram function only
    cvtColor(frameFinal, frameFinalDuplicate1, COLOR_RGB2BGR);   //used in histrogram function only
}

void Histrogram()
{
    histrogramLane.resize(CameraWIDTH);
    histrogramLane.clear();

    for (int i = 0; i < CameraWIDTH; i++) // frame.size().width = 400
    {
        ROILane = frameFinalDuplicate(Rect(i, 280, 1, 200)); 
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0]));
        ROILaneEnd = frameFinalDuplicate1(Rect(i, 0, 1, CameraHeight));
        divide(255, ROILaneEnd, ROILaneEnd);
        histrogramLaneEnd.push_back((int)(sum(ROILaneEnd)[0]));
    }
    laneEnd = sum(histrogramLaneEnd)[0];
    cout << "Lane END = " << endl;
}

void LaneFinder()
{
    vector<int>::iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 270);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr);

    vector<int>::iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() + 370, histrogramLane.end());
    RightLanePos = distance(histrogramLane.begin(), RightPtr);

    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 480), Scalar(0, 255, 0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 480), Scalar(0, 255, 0), 2);
}

void LaneCenter()
{
    laneCenter = (RightLanePos - LeftLanePos) / 2 + LeftLanePos;
    frameCenter = 285;

    line(frameFinal, Point2f(laneCenter, 0), Point2f(laneCenter, 480), Scalar(0, 255, 0), 3);
    line(frameFinal, Point2f(frameCenter, 0), Point2f(frameCenter, 480), Scalar(255, 0, 0), 3);

    Result = laneCenter - frameCenter;
}

int main()
{
    setup();
    const string gpio_chip = "gpiochip0";
    const unsigned int pins[] = {5, 6, 19, 26}; // 7 6 5 4 in the arduino, 5 6 19 26 in meaning GPIO 5,6,19,26 not the pins
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
        auto start = chrono::system_clock::now();
        captureFrames();
        Perspective();
        Threshold();
        Histrogram();
        LaneFinder();
        LaneCenter();
 
        if (laneEnd > 3000) // stop
        {
            lines.set_values({0, 1, 1, 1});
            cout << "Lane End" << endl;
        }
        if (Result == 0)
        {
            lines.set_values({0, 0, 0, 0});
            cout << "Forward" << endl;
        }

        else if (Result > 0 && Result < 10)
        {
            lines.set_values({0, 0, 0, 1}); // decimal = 1
            cout << "Right1" << endl;
        }

        else if (Result >= 10 && Result < 20)
        {
            lines.set_values({0, 0, 1, 0}); // decimal = 2
            cout << "Right2" << endl;
        }

        else if (Result >= 20)
        {
            lines.set_values({0, 0, 1, 1}); // decimal = 3
            cout << "Right3" << endl;
        }

        else if (Result < 0 && Result > -10)
        {
            lines.set_values({0, 1, 0, 0}); // decimal =4
            cout << "Left1" << endl;
        }

        else if (Result <= -10 && Result > -20)
        {
            lines.set_values({0, 1, 0, 1}); // decimal = 5
            cout << "Left2" << endl;
        }

        else if (Result <= -20)
        {
            lines.set_values({0, 1, 1, 0}); // decimal = 6
            cout << "Left3" << endl;
        }
        if (laneEnd > 3000)
        {
            ss.str(" ");
            ss.clear();
            ss << " Lane End";
            putText(frame, ss.str(), Point2f(1, 50), 0, 1, Scalar(255, 0, 0), 2);
        }

        else if (Result == 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Forward";
            putText(frame, ss.str(), Point2f(1, 50), 0, 1, Scalar(0, 0, 255), 2);
        }

        else if (Result > 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << "bMove Right";
            putText(frame, ss.str(), Point2f(1, 50), 0, 1, Scalar(0, 0, 255), 2);
        }

        else if (Result < 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Left";
            putText(frame, ss.str(), Point2f(1, 50), 0, 1, Scalar(0, 0, 255), 2);
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

        auto key = waitKey(1);
        auto end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;

        float t = elapsed_seconds.count();
        int FPS = 1 / t;
        cout << "FPS = " << FPS << endl;
    }

    return 0;
}
