#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <wiringPi.h>

using namespace cv;
using namespace std;



Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, ROILane;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result;

vector<int> histrogramLane;
Point2f Source[] = {Point2f(40,145), Point2f(360,145), Point2f(10,195), Point2f(390,195)};
Point2f Destination[] = {Point2f(100,0), Point2f(280,0), Point2f(100,240), Point2f(280,240)};
VideoCapture cap(0);

stringstream ss;

void setup() {
    cap.set(CAP_PROP_FRAME_WIDTH, 640); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FRAME_HEIGHT, 480); // Adjust based on your webcam specifications
    cap.set(CAP_PROP_FPS, 60);  // Adjust as needed
    cap.set(CAP_PROP_BRIGHTNESS, 50);
    cap.set(CAP_PROP_CONTRAST, 50);
    cap.set(CAP_PROP_SATURATION, 50);
    cap.set(CAP_PROP_GAIN, 50);
}

void captureFrames() {
    if (cap.read(frame)) {
        cvtColor(frame, frame, COLOR_BGR2RGB);
    }
}

void Perspective() {
    line(frame, Source[0], Source[1], Scalar(0,0,255), 2);
    line(frame, Source[1], Source[3], Scalar(0,0,255), 2);
    line(frame, Source[3], Source[2], Scalar(0,0,255), 2);
    line(frame, Source[2], Source[0], Scalar(0,0,255), 2);

    Matrix = getPerspectiveTransform(Source, Destination);
    warpPerspective(frame, framePers, Matrix, Size(400,240));
}

void Threshold() {
    cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
    inRange(frameGray, 200, 255, frameThresh);
    Canny(frameGray, frameEdge, 900, 900, 3, false);
    add(frameThresh, frameEdge, frameFinal);
    cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);   //used in histrogram function only
}

void Histrogram() {
    histrogramLane.resize(400);
    histrogramLane.clear();
    for(int i=0; i<400; i++) {
        ROILane = frameFinalDuplicate(Rect(i,140,1,100));
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0]));
    }
}

void LaneFinder() {
    vector<int>::iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 150);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr);

    vector<int>::iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() + 250, histrogramLane.end());
    RightLanePos = distance(histrogramLane.begin(), RightPtr);

    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255,0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0,255,0), 2);
}

void LaneCenter() {
    laneCenter = (RightLanePos - LeftLanePos) / 2 + LeftLanePos;
    frameCenter = 188;

    line(frameFinal, Point2f(laneCenter,0), Point2f(laneCenter,240), Scalar(0,255,0), 3);
    line(frameFinal, Point2f(frameCenter,0), Point2f(frameCenter,240), Scalar(255,0,0), 3);

    Result = laneCenter - frameCenter;
}

int main() {
    setup();
    wiringPiSetup();
    pinMode(0, OUTPUT);
    if (!cap.isOpened()) {
        cerr << "Error opening webcam." << endl;
        return -1;
    }

    cout << "Webcam is opened" << endl;

    while (1) {
        auto start = chrono::system_clock::now();
        captureFrames();
        Perspective();
        Threshold();
        Histrogram();
        LaneFinder();
        LaneCenter();
        
        ss.str(" ");
        ss.clear();
        ss << "Result = " << Result;
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);

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
