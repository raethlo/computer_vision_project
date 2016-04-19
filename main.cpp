
#include <iostream>
#include <fstream>
#include <sstream>

#include <cv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"

using namespace std;
using namespace cv;
using namespace cv::face;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator);
void detectFaces(CascadeClassifier face_cascade,Ptr<face::FaceRecognizer> emotion_classifier, Mat frame);
Ptr<face::FaceRecognizer> trainEmotionClassifier(CascadeClassifier face_cascade);

String cascade_dir_path = "/home/raethlo/libs/opencv-3.1.0/data/haarcascades/";
Size img_size;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ',') {
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            Mat m = imread(path, 1);
            Mat m2;
            cvtColor(m, m2, CV_RGB2GRAY);

            images.push_back(m2);
            labels.push_back(atoi(classlabel.c_str()));
        } else {
            cout << "Couldn't read file: " << path << endl;
        }

        img_size = images[0].size();

        for(int i=0; i < images.size(); i++)
            resize(images[i],images[i],images[0].size(), 0.5, 0.5);
    }
}

// model0->save("eigenfaces_at.yml");
void detectFaces(CascadeClassifier face_cascade, Ptr<face::FaceRecognizer> emotion_recognizer, Mat frame)
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
        Mat scaledFaceROI;
        resize(faceROI, scaledFaceROI, img_size, 0.5, 0.5);

        int prediction = emotion_recognizer->predict(scaledFaceROI);
        cout << "Could be: " << prediction << endl;

        rectangle(frame, point1, point2, cvScalar(255, 255, 0), 2);
    }
}

void cutFacesFromImages(CascadeClassifier face_cascade, vector<Mat> images, vector<Mat> &faces){
    vector<Rect> fcs;

    for(int i = 0; i< images.size(); i++){
        equalizeHist(images[i], images[i]);
        face_cascade.detectMultiScale(images[i], fcs, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
        Mat faceROI = images[i](fcs[0]);

        faces.push_back(faceROI);
    }
}

Ptr<face::FaceRecognizer> trainEmotionClassifier(CascadeClassifier face_cascade)
{
    // Get the path to your CSV
    string fn_csv = "/home/raethlo/Developer/cpp/computer_vision_project/emotions.csv";
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    vector<Mat> faces;
    // Read in the data. This can fail if no valid
    // input filename is given.
    cout << "\"" + fn_csv + "\"" << endl;
    read_csv(fn_csv, images, labels);
    cutFacesFromImages(face_cascade, images, faces);

    Ptr<face::FaceRecognizer> emotion_classifier = createEigenFaceRecognizer(25);
    emotion_classifier->train(images, labels);
    return emotion_classifier;
}

int main(int argc, char** argv)
{
    CascadeClassifier face_classifier;
    CascadeClassifier smile_classifier;
    Ptr<FaceRecognizer> emotion_recognizer;

    String face_cascade_name = "haarcascade_frontalface_default.xml";
    String smile_cascade_name = "haarcascade_smile.xml";
    String window_name = "Face detection";
    Mat frame;
    Mat face;


    if (!face_classifier.load(cascade_dir_path + face_cascade_name)) {
        printf("couldt load haar cascade data\n");
        return 1;
    }

    if (!smile_classifier.load(cascade_dir_path + smile_cascade_name)) {
        printf("couldt load haar cascade data\n");
        return 1;
    }

    try {
        printf("loading emotion classifier");
        emotion_recognizer = trainEmotionClassifier(face_classifier);
        emotion_recognizer->save("er_eigenfaces.yml");
    } catch (cv::Exception& e) {
        cerr << "Error opening file. Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
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

        detectFaces(face_classifier, emotion_recognizer, frame);
        imshow(window_name, frame);
//        imshow("face", face);

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

