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
Mat frame, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, ROILane;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd;

vector<int> histrogramLane;

Point2f Source[] = {Point2f(60, 135), Point2f(200, 135), Point2f(30, 210), Point2f(230, 210)};
Point2f Destination[] = {Point2f(85, 0), Point2f(180, 0), Point2f(85, 240), Point2f(180, 240)};

VideoCapture cap(0);
stringstream ss;

// Machine Learning variables
CascadeClassifier Stop_Cascade, Object_Cascade, Traffic_Cascade, Uturn_Cascade;
Mat frame_Signs, RoI_Signs, gray_Signs, RoI_Object, frame_Object, gray_Object, frame_clone;
vector<Rect> Stop, Object, Traffic, Uturn;
int dist_Stop, dist_Object, dist_Traffic, dist_Uturn;

void setup()
{
    cap.set(CAP_PROP_FRAME_WIDTH, CameraWIDTH);   // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FRAME_HEIGHT, CameraHeight); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FPS, 100);
    cap.set(CAP_PROP_BRIGHTNESS, 15);
    cap.set(CAP_PROP_CONTRAST, 10);    // 50
    cap.set(CAP_PROP_SATURATION, 100); // 50
    cap.set(CAP_PROP_GAIN, 20);        // 10
}

void captureFrames()
{
    if (cap.read(frame))
    {
        cvtColor(frame, frame, COLOR_BGR2RGB);
        frame_Object = frame.clone();
        frame_Signs = frame.clone();
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
    static Mat Matrix = getPerspectiveTransform(Source, Destination);
    warpPerspective(frame, framePers, Matrix, Size(CameraWIDTH, CameraHeight));
}

void Threshold()
{
    cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
    inRange(frameGray, 250, 255, frameThresh);
    Canny(frameGray, frameEdge, 900, 900, 3, false);
    add(frameThresh, frameEdge, frameFinal);
    cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR); // used in histrogram function
}

void Histrogram()
{
    histrogramLane.resize(CameraWIDTH);
    histrogramLane.clear();
    for (int i = 0; i < CameraWIDTH; i++)
    {
        ROILane = frameFinalDuplicate(Rect(i, 140, 1, 100));
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0]));
    }
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
int loadCascadeFile()
{
    if (!Stop_Cascade.load(".//MachineLearning//Stop_cascade.xml"))
    {
        cerr << "Unable to open stop cascade file" << endl;
    }
    if (!Object_Cascade.load(".//MachineLearning//Object_cascade.xml"))
    {
        cerr << "Unable to open Object cascade file" << endl;
    }
    if (!Traffic_Cascade.load(".//MachineLearning//Trafficc_cascade.xml"))
    {
        cerr << "Unable to open Traffic cascade file" << endl;
    }
    if (!Uturn_Cascade.load(".//MachineLearning//uTurn_cascade.xml"))
    {
        cerr << "Unable to open uTurn cascade file" << endl;
    }
    return 1;
}

void Signs_detection()
{
    RoI_Signs = frame_Signs(Rect(100, 0, 200, 240));
    cvtColor(RoI_Signs, gray_Signs, COLOR_RGB2GRAY);
    equalizeHist(gray_Signs, gray_Signs);

    Uturn_Cascade.detectMultiScale(gray_Signs, Uturn);

    for (size_t i = 0; i < Uturn.size(); i++)
    {
        Point P1_Uturn(Uturn[i].x, Uturn[i].y);
        Point P2_Uturn(Uturn[i].x + Uturn[i].width, Uturn[i].y + Uturn[i].height);

        rectangle(RoI_Signs, P1_Uturn, P2_Uturn, Scalar(0, 0, 255), 1);
        putText(RoI_Signs, "uTurn Sign", Point(P1_Uturn.x, P1_Uturn.y - 2), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 255), 1);
        dist_Uturn = (-0.625) * (P2_Uturn.x - P1_Uturn.x) + 55.5;
        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Uturn << "cm";
        putText(RoI_Signs, ss.str(), Point2f(1, 130), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 255), 1);
    }

    Stop_Cascade.detectMultiScale(gray_Signs, Stop);

    for (size_t i = 0; i < Stop.size(); i++)
    {
        Point P1_Stop(Stop[i].x, Stop[i].y);
        Point P2_Stop(Stop[i].x + Stop[i].width, Stop[i].y + Stop[i].height);

        rectangle(RoI_Signs, P1_Stop, P2_Stop, Scalar(0, 255, 0), 1);
        putText(RoI_Signs, "Stop Sign", Point(P1_Stop.x, P1_Stop.y - 2), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 255, 0), 1);
        dist_Stop = (-0.625) * (P2_Stop.x - P1_Stop.x) + 58.25;
        // dist_Stop = (P2_Stop.x - P1_Stop.x) ;
        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Stop << "cm";
        putText(RoI_Signs, ss.str(), Point2f(1, 150), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 255, 0), 1);
    }

    Traffic_Cascade.detectMultiScale(gray_Signs, Traffic);

    for (size_t i = 0; i < Traffic.size(); i++)
    {
        Point P1_Traffic(Traffic[i].x, Traffic[i].y);
        Point P2_Traffic(Traffic[i].x + Traffic[i].width, Traffic[i].y + Traffic[i].height);

        rectangle(RoI_Signs, P1_Traffic, P2_Traffic, Scalar(255, 0, 0), 1);
        putText(RoI_Signs, "Traffic Sign", Point(P1_Traffic.x, P1_Traffic.y - 2), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(255, 0, 0), 1);
        dist_Traffic = -0.43478*(P2_Traffic.x - P1_Traffic.x)+39.5217;
        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Traffic << "cm";
        putText(RoI_Signs, ss.str(), Point2f(1, 170), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(255, 0, 0), 1);
    }
}

void Object_detection()
{

    RoI_Object = frame_Object(Rect(0, 0, 300, 240));
    cvtColor(RoI_Object, gray_Object, COLOR_RGB2GRAY);
    equalizeHist(gray_Object, gray_Object);
    Object_Cascade.detectMultiScale(gray_Object, Object);

    for (size_t i = 0; i < Object.size(); i++)
    {
        Point P1(Object[i].x, Object[i].y);
        Point P2(Object[i].x + Object[i].width, Object[i].y + Object[i].height);

        rectangle(RoI_Object, P1, P2, Scalar(0, 0, 255), 1);
        putText(RoI_Object, "Object", Point(P1.x, P1.y - 2), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255, 255), 1);
        dist_Object = (-1.23) * (P2.x - P1.x) + 89.538;
        // dist_Object= (P2.x - P1.x);

        ss.str(" ");
        ss.clear();
        ss << "D = " << dist_Object << "cm";
        putText(RoI_Object, ss.str(), Point2f(1, 130), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
    }
}

void displayMonitors()
{
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
    namedWindow("Object", WINDOW_KEEPRATIO);
    moveWindow("Object", 640, 580);
    resizeWindow("Object", 640, 480);
    imshow("Object", RoI_Object);
    namedWindow("Sign", WINDOW_KEEPRATIO);
    moveWindow("Sign", 1280, 580);
    resizeWindow("Sign", 640, 480);
    imshow("Sign", RoI_Signs);
    waitKey(1);
}

int main()
{
    setup();
    loadCascadeFile();
    const string gpio_chip = "gpiochip0";
    const unsigned int pins[] = {5, 6, 19, 26}; // 5 4 3 2 in the arduino, 5 6 19 26 in meaning GPIO 5,6,19,26 not the pins
    chip chip(gpio_chip);
    line_bulk lines(chip.get_lines({pins[0], pins[1], pins[2], pins[3]}));
    lines.request({{}, line_request::DIRECTION_OUTPUT, {}});

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
        Object_detection();
        Signs_detection();

        if (dist_Traffic > 10 && dist_Traffic < 30)
        {
            ss.str(" ");
            ss.clear();
            ss << "Traffic Light";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 1, 1, 1}); // decimal = 8
            cout << "Traffic Light" << endl;
            dist_Traffic = 0;
        }
        else if (dist_Stop > 15 && dist_Stop < 35)
        {
            ss.str(" ");
            ss.clear();
            ss << "Stop Sign";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 0, 0, 0}); // decimal = 8
            cout << "Stop Sign" << endl;
            dist_Stop = 0;
        }
        else if (dist_Uturn > 0)
        {
            ss.str(" ");
            ss.clear();
            ss << "Uturn Sign";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({0, 1, 1, 1});
            cout << "Uturn Sign" << endl;
            dist_Uturn = 0;
        }
        else if (dist_Object > 10 && dist_Object < 80)
        {
            ss.str(" ");
            ss.clear();
            ss << "Object";
            putText(frame, ss.str(), Point2f(5, 30), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 0, 0, 1}); // decimal = 8
            cout << "Object" << endl;
            dist_Object = 0;
        }
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
    
        displayMonitors();
        while (waitKey(1) != -1)
        { // Emergency stop
            ss.str(" ");
            ss.clear();
            ss << "Emergency stop!";
            putText(frame, ss.str(), Point2f(5, 15), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);
            lines.set_values({1, 1, 1, 1});
            cout << "Emergency stop! press s to continue" << endl;
            if (cin.get() == 's')
                break;
        }

        double fps = getTickFrequency() / (getTickCount() - start);
        cout << "FPS : " << fps << endl;
    }
    return 0;
}