// #include <opencv2/opencv.hpp>
// #include <iostream>
// #include <chrono>
// #define Point2f cv::Point2f

// // set up the camera object as a global variable
// cv::Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate,ROILane;
// int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result;

// std::vector<int> histrogramLane;
// Point2f Source[] = {Point2f(40,145),Point2f(360,145),Point2f(10,195), Point2f(390,195)};

// Point2f Destination[] = {Point2f(100,0),Point2f(280,0),Point2f(100,240), Point2f(280,240)};
// cv::VideoCapture cap(0);
// std::stringstream ss;
// void setup(){
//     cap.set(cv::CAP_PROP_FRAME_WIDTH, 400);
//     cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
//     cap.set(cv::CAP_PROP_FPS, 0);  // Set desired frame rate (adjust as needed)
//     cap.set(cv::CAP_PROP_BRIGHTNESS, 50);
//     cap.set(cv::CAP_PROP_CONTRAST, 50);
//     cap.set(cv::CAP_PROP_SATURATION, 50);
//     cap.set(cv::CAP_PROP_GAIN, 50);
//      }
// void captureFrames() {
//         if (cap.read(frame)) {
//             cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
//         }
// }
// void Perspective()
// {
// 	line(frame,Source[0], Source[1], cv::Scalar(0,0,255), 2);
// 	line(frame,Source[1], Source[3],  cv::Scalar(0,0,255), 2);
// 	line(frame,Source[3], Source[2],  cv::Scalar(0,0,255), 2);
// 	line(frame,Source[2], Source[0],  cv::Scalar(0,0,255), 2);
	
	
// 	Matrix = getPerspectiveTransform(Source, Destination);
// 	warpPerspective(frame, framePers, Matrix, cv::Size(400,240));
// }
// void Threshold()
// {
// 	cvtColor(framePers, frameGray, cv::COLOR_RGB2GRAY);
// 	inRange(frameGray, 200, 255, frameThresh);
// 	Canny(frameGray,frameEdge, 900, 900, 3, false);
// 	add(frameThresh, frameEdge, frameFinal);
// 	cvtColor(frameFinal, frameFinal, cv::COLOR_GRAY2RGB);
// 	cvtColor(frameFinal, frameFinalDuplicate, cv::COLOR_RGB2BGR);   //used in histrogram function only
	
// }
// void Histrogram()
// {
//     histrogramLane.resize(400);
//     histrogramLane.clear();
//     for(int i=0; i<400; i++)       //frame.size().width = 400
//     {
// 	ROILane = frameFinalDuplicate(cv::Rect(i,140,1,100));
// 	divide(255, ROILane, ROILane);
// 	histrogramLane.push_back((int)(sum(ROILane)[0])); 
//     }
// }
// void LaneFinder()
// {
//     std::vector<int>:: iterator LeftPtr;
//     LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 150);
//     LeftLanePos = distance(histrogramLane.begin(), LeftPtr); 
    
//     std::vector<int>:: iterator RightPtr;
//     RightPtr = max_element(histrogramLane.begin() +250, histrogramLane.end());
//     RightLanePos = distance(histrogramLane.begin(), RightPtr);
    
//     line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), cv::Scalar(0, 255,0), 2);
//     line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), cv::Scalar(0,255,0), 2); 
// }

// void LaneCenter()
// {
//     laneCenter = (RightLanePos-LeftLanePos)/2 +LeftLanePos;
//     frameCenter = 188;
    
//     line(frameFinal, Point2f(laneCenter,0), Point2f(laneCenter,240), cv::Scalar(0,255,0), 3);
//     line(frameFinal, Point2f(frameCenter,0), Point2f(frameCenter,240), cv::Scalar(255,0,0), 3);

//     Result = laneCenter-frameCenter;
// }
// int main() {

//     setup();
//     if (!cap.isOpened()) {
//         std::cerr << "Error opening webcam." << std::endl;
//         return -1;
//     }
//     std::cout<<"Webcam is opened"<<std::endl;
//     while (1) {
//     auto start = std::chrono::system_clock::now();
//     captureFrames();
//     Perspective();
//     Threshold();
//     Histrogram();
//     LaneFinder();
//     LaneCenter();
//     ss.str(" ");
//     ss.clear();
//     ss<<"Result = "<<Result;
//     putText(frame, ss.str(), Point2f(1,50), 0,1, cv::Scalar(0,0,255), 2);
    
    
//     cv::namedWindow("orignal", cv::WINDOW_KEEPRATIO);
//     cv::moveWindow("orignal", 0, 100);
//     cv::resizeWindow("orignal", 640, 480);
//     imshow("orignal", frame);
    
//     cv::namedWindow("Perspective", cv::WINDOW_KEEPRATIO);
//     cv::moveWindow("Perspective", 640, 100);
//     cv::resizeWindow("Perspective", 640, 480);
//     imshow("Perspective", framePers);
    
//     cv::namedWindow("Final", cv::WINDOW_KEEPRATIO);
//     cv::moveWindow("Final", 1280, 100);
//     cv::resizeWindow("Final", 640, 480);
//     imshow("Final", frameFinal);
//        auto key = cv::waitKey(1);
//       auto end = std::chrono::system_clock::now();
//     std::chrono::duration<double> elapsed_seconds = end-start;
    
//     float t = elapsed_seconds.count();
//     int FPS = 1/t;
//     std::cout<<"FPS = "<<FPS<<std::endl;
//     }
  
//     return 0;     
// }