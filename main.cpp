
#include <iostream>
#include <cv.hpp>

using namespace std;
using namespace cv;

// f() header
void detectFaces(CascadeClassifier face_cascade, CascadeClassifier smile_cascade, Mat frame);

String cascade_dir_path = "/home/raethlo/libs/opencv-3.1.0/data/haarcascades/";

int main(int argc, char** argv)
{
    CascadeClassifier face_classifier;
    CascadeClassifier smile_classifier;

    String face_cascade_name = "haarcascade_frontalface_default.xml";
    String smile_cascade_name = "haarcascade_smile.xml";
    String window_name = "Face detection";
    Mat frame;


    if (!face_classifier.load(cascade_dir_path + face_cascade_name)) {
        printf("couldt load haar cascade data\n");
        return 1;
    }

    if (!smile_classifier.load(cascade_dir_path + smile_cascade_name)) {
        printf("couldt load haar cascade data\n");
        return 1;
    }

    //capture = cvCaptureFromCAM(0);
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        printf("couldt load cam\n");
        waitKey();
        return 1;
    }

    while(true) {
        capture >> frame;
        if (frame.empty()) { cout << "Empty frame!" << endl; break; }

        detectFaces(face_classifier, smile_classifier, frame);
        imshow(window_name, frame);

        if (waitKey(30) == 27)
        {
            cout << "esc key is pressed by user" << endl;
            destroyWindow(window_name);
            break;
        }
    }

    waitKey(0);
    return 0;
}

void detectFaces(CascadeClassifier face_cascade, CascadeClassifier smile_cascade, Mat frame)
{
    vector<Rect> faces;
    Mat frame_gray;

    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

    for (int i = 0; i < faces.size(); i++) {
        Point point1(faces[i].x, faces[i].y);
        Point point2(faces[i].x + faces[i].width, faces[i].y + faces[i].height);
        Mat faceROI = frame_gray(faces[i]);

        rectangle(frame, point1, point2, cvScalar(0, 255, 0), 2);
    }

}
