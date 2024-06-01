#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <gpiod.hpp>
#include <vector>

#define CameraWIDTH 400
#define CameraHeight 240

using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace gpiod; // line class changed to lineGPIO

// Image Processing variables
Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, frameFinalDuplicate1;
Mat ROILane, ROILaneEnd;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd;

vector<int> histrogramLane;
vector<int> histrogramLaneEnd;

// Point2f Source[] = {Point2f(40, 135), Point2f(230, 135), Point2f(10, 230), Point2f(260, 230)};
// Point2f Destination[] = {Point2f(65, 0), Point2f(200, 0), Point2f(65, 240), Point2f(200, 240)};

Point2f Source[] = {Point2f(60, 135), Point2f(200, 135), Point2f(30, 210), Point2f(230, 210)};
Point2f Destination[] = {Point2f(85, 0), Point2f(180, 0), Point2f(85, 240), Point2f(180, 240)};


VideoCapture cap(0);

stringstream ss;

// Machine Learning variables
CascadeClassifier Stop_Cascade, Object_Cascade, Traffic_Cascade;
Mat frame_Stop, RoI_Stop, gray_Stop, frame_Object, RoI_Object, gray_Object, frame_Traffic, RoI_Traffic, gray_Traffic;
vector<Rect> Stop, Object, Traffic;
int dist_Stop, dist_Object, dist_Traffic;

void setup()
{
    cap.set(CAP_PROP_FRAME_WIDTH, CameraWIDTH);   // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FRAME_HEIGHT, CameraHeight); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FPS, 100);
    cap.set(CAP_PROP_BRIGHTNESS, 15);
    cap.set(CAP_PROP_CONTRAST, 10);     // 50
    cap.set(CAP_PROP_SATURATION, 100); // 50
    cap.set(CAP_PROP_GAIN, 20);        // 10
}

void captureFrames()
{
    if (cap.read(frame))
    {
        cvtColor(frame, frame_Stop, COLOR_BGR2RGB);
        cvtColor(frame, frame_Object, COLOR_BGR2RGB);
        cvtColor(frame, frame_Traffic, COLOR_BGR2RGB);
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
    for (int i = 0; i < CameraWIDTH; i++)
    {
        ROILane = frameFinalDuplicate(Rect(i, 140, 1, 100));

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
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr);

    vector<int>::iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() + 140, histrogramLane.end() - 20);
    RightLanePos = distance(histrogramLane.begin(), RightPtr);

    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255, 0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0, 255, 0), 2);
}

void LaneCenter()
{
    laneCenter = (RightLanePos - LeftLanePos) / 2 + LeftLanePos;
    frameCenter = 133;
    line(frameFinal, Point2f(laneCenter, 0), Point2f(laneCenter, 240), Scalar(0, 255, 0), 3);
    line(frameFinal, Point2f(frameCenter, 0), Point2f(frameCenter, 240), Scalar(255, 0, 0), 3);
    Result = laneCenter - frameCenter;
}

void Stop_detection()
{
    if (!Stop_Cascade.load("//home//pi//New_project//MachineLearning//Stop_cascade.xml"))
    {
        cerr << "Unable to open stop cascade file" << endl;
    }

    RoI_Stop = frame_Stop(Rect(100,0,200,240));
    cvtColor(RoI_Stop, gray_Stop, COLOR_RGB2GRAY);
    equalizeHist(gray_Stop, gray_Stop);
    Stop_Cascade.detectMultiScale(gray_Stop, Stop);

    for (int i = 0; i < Stop.size(); i++)
    {
        Point P1(Stop[i].x, Stop[i].y);
        Point P2(Stop[i].x + Stop[i].width, Stop[i].y + Stop[i].height);

        rectangle(RoI_Stop, P1, P2, Scalar(0, 0, 255), 1);
        putText(RoI_Stop, "Stop Sign", P1, FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255, 255), 1);
        dist_Stop = (-3.33333) * (P2.x - P1.x) + 170;
        // dist_Stop = (P2.x - P1.x) ;
        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Stop << "cm";
        putText(RoI_Stop, ss.str(), Point2f(1, 130), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
    }
}

void Object_detection()
{
    if (!Object_Cascade.load("//home//pi//New_project//MachineLearning//Object_cascade.xml"))
    {
        cerr << "Unable to open Object cascade file" << endl;
    }

    RoI_Object = frame_Object(Rect(0, 0, 300, 200));
    cvtColor(RoI_Object, gray_Object, COLOR_RGB2GRAY);
    equalizeHist(gray_Object, gray_Object);
    Object_Cascade.detectMultiScale(gray_Object, Object);

    for (int i = 0; i < Object.size(); i++)
    {
        Point P1(Object[i].x, Object[i].y);
        Point P2(Object[i].x + Object[i].width, Object[i].y + Object[i].height);

        rectangle(RoI_Object, P1, P2, Scalar(0, 0, 255), 1);
        putText(RoI_Object, "Object", P1, FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255, 255), 1);
        dist_Object = (-1.625) * (P2.x - P1.x) + 165.75;
        // dist_Object= (P2.x - P1.x) ;

        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Object << "cm";
        putText(RoI_Object, ss.str(), Point2f(1, 130), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
    }
}

// void Traffic_detection()
// {
//     if (!Traffic_Cascade.load("//home//pi//Desktop//MACHINE LEARNING//Trafficc_cascade.xml"))
//     {
//         cerr << "Unable to open Traffic cascade file" << endl;
//     }

//     RoI_Traffic = frame_Traffic(Rect(200, 0, 200, 140));
//     cvtColor(RoI_Traffic, gray_Traffic, COLOR_RGB2GRAY);
//     equalizeHist(gray_Traffic, gray_Traffic);
//     Traffic_Cascade.detectMultiScale(gray_Traffic, Traffic);

//     for (int i = 0; i < Traffic.size(); i++)
//     {
//         Point P1(Traffic[i].x, Traffic[i].y);
//         Point P2(Traffic[i].x + Traffic[i].width, Traffic[i].y + Traffic[i].height);

//         rectangle(RoI_Traffic, P1, P2, Scalar(0, 0, 255), 2);
//         putText(RoI_Traffic, "Traffic Light", P1, FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255), 2);
//         dist_Traffic = (-1.07) * (P2.x - P1.x) + 102.597;

//         ss.str(" ");
//         ss.clear();
//         ss << "D = " << P2.x - P1.x << "cm";
//         putText(RoI_Traffic, ss.str(), Point2f(1, 130), 0, 1, Scalar(0, 0, 255), 2);
//     }
// }

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
        Stop_detection();
        Object_detection();
        // Traffic_detection();

        if (dist_Stop > 15 && dist_Stop < 40)
        {
            ss.str(" ");
            ss.clear();
            ss << "Stop Sign";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 0, 0, 0}); // decimal = 8
            cout << "Stop Sign" << endl;
            dist_Stop = 0;

            goto Stop_Sign;
        }
        else if (dist_Object > 20 && dist_Object < 40)
        {
            ss.str(" ");
            ss.clear();
            ss << "Object";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 0, 0, 1}); // decimal = 8
            cout << "Object" << endl;
            dist_Object = 0;

            goto Object;
        }
        // if (laneEnd > 7000 && laneEnd<10000) // stop
        // {
        //     ss.str(" ");
        //     ss.clear();
        //     ss << " Lane End";
        //     putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
        //     lines.set_values({0, 1, 1, 1});
        //     // cout << "Lane End" << endl;
        // }
        else if (Result == 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Result = " << Result << " Move Forward";
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
    Stop_Sign:
    Object:

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
        namedWindow("Stop Sign", WINDOW_KEEPRATIO);
        moveWindow("Stop Sign", 1280, 580);
        resizeWindow("Stop Sign", 640, 480);
        imshow("Stop Sign", RoI_Stop);

        namedWindow("Object", WINDOW_KEEPRATIO);
        moveWindow("Object", 640, 580);
        resizeWindow("Object", 640, 480);
        imshow("Object", RoI_Object);

        // namedWindow("Traffic", WINDOW_KEEPRATIO);
        // moveWindow("Traffic", 0, 580);
        // resizeWindow("Traffic", 640, 480);
        // imshow("Traffic", RoI_Traffic);
        // waitKey(1);
        while(waitKey(1) != -1){ // stop
            ss.str(" ");
            ss.clear();
            ss << "Emergency stop!";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 1, 1, 1});
            cout << "Emergency stop!" << endl;
            if (cin.get() == 's')
                break;
        }

        double fps = getTickFrequency() / (getTickCount() - start);
        cout << "FPS : " << fps << endl;
    }
    return 0;
}