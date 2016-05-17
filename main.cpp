
#include <iostream>
#include <fstream>
#include <sstream>

#include <cv.hpp>
#include <csignal>
#include "opencv2/face.hpp"

using namespace std;
using namespace cv;
using namespace cv::face;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator);
void detectFaces(CascadeClassifier face_cascade,Ptr<face::FaceRecognizer> emotion_classifier, Mat frame);
Ptr<face::FaceRecognizer> trainEmotionClassifier(CascadeClassifier face_cascade);

void cutFaceROI(Mat inputFace, Mat& outputFace);

string emotionNameFromLabel(int label);

string positiveNegativeName(int value);
int toPositiveNegative(int prediction);

String cascade_dir_path = "/home/raethlo/libs/opencv-3.1.0/data/haarcascades/";

// todo lOCAL BINARY PATTERNS

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

        images[0].size();

    }
}

// model0->save("eigenfaces_at.yml");
void detectFaces(CascadeClassifier face_cascade, Ptr<face::FaceRecognizer> emotion_recognizer, Mat frame)
{
    vector<Rect> faces;
    Mat frame_gray;

    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(200, 200));

    for (int i = 0; i < faces.size(); i++) {
        Point point1(faces[i].x, faces[i].y);
        Point point2(faces[i].x + faces[i].width, faces[i].y + faces[i].height);


        Mat faceROI = frame_gray(faces[i]);
        Mat scaledFaceROI;
        Mat faceTrimmed;

        cutFaceROI(faceROI, faceTrimmed);

        resize(faceTrimmed, scaledFaceROI, Size(500,500));

        int prediction = emotion_recognizer->predict(scaledFaceROI);

        putText(frame, emotionNameFromLabel(prediction), point1, FONT_HERSHEY_TRIPLEX, 2.0, Scalar(0,0,255));

        rectangle(frame, point1, point2, cvScalar(255, 255, 0), 2);
    }
}

void cutFaceROI(Mat inputFace, Mat& outputFace){
    int xToTrim = inputFace.cols * 0.2;
    int yToTrim = inputFace.rows * 0.15;
    outputFace = inputFace(Rect(xToTrim, yToTrim, inputFace.cols * 0.6, inputFace.rows * 0.85));
}

void cutFacesFromImages(CascadeClassifier face_cascade, vector<Mat> images, vector<Mat> &faces){
    vector<Rect> fcs;

    for(int i = 0; i< images.size(); i++){
        equalizeHist(images[i], images[i]);
        face_cascade.detectMultiScale(images[i], fcs, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(200, 200));
        Mat faceROI = images[i](fcs[0]);
        Mat faceTrimmed;
        Mat resized;

        cutFaceROI(faceROI, faceTrimmed);

        resize(faceTrimmed, resized, Size(500,500));

        faces.push_back(faceTrimmed);
    }

}

string emotionNameFromLabel(int label) {
    // There should be only one entry and the number will range from 0-7
    // (i.e. 0=neutral, 1=anger, 2=contempt, 3=disgust, 4=fear, 5=happy, 6=sadness, 7=surprise)
    switch(label) {
        case 0:
            return "neutral";
        case 1:
            return "anger";
        case 2:
            return "contempt";
        case 3:
            return "disgust";
        case 4:
            return "fear";
        case 5:
            return "happy";
        case 6:
            return "sadness";
        case 7:
            return "surprise";
    }
}

string positiveNegativeName(int value) {
    if(value == 0)
        return "negative";

    return "positive";
}

int toPositiveNegative(int prediction) {
    if((prediction == 0) || (prediction == 2) || (prediction == 5))
        return 1;

    return 0;
}

Ptr<face::FaceRecognizer> trainEmotionClassifier(CascadeClassifier face_cascade)
{
    // Get the path to your CSV
    string fn_csv = "/home/raethlo/Developer/cpp/computer_vision_project/emotions.csv";
//    string fn_csv = "/home/raethlo/Developer/cpp/computer_vision_project/emotions_happysad.csv";
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    vector<Mat> faces;

    // Read in the data. This can fail if no valid
    // input filename is given.
    cout << "\"" + fn_csv + "\"" << endl;
    read_csv(fn_csv, images, labels);
    cutFacesFromImages(face_cascade, images, faces);

    Ptr<face::FaceRecognizer> emotion_classifier = createFisherFaceRecognizer();
//    Ptr<face::FaceRecognizer> emotion_classifier = createEigenFaceRecognizer();
//    Ptr<face::FaceRecognizer> emotion_classifier = createLBPHFaceRecognizer();
    emotion_classifier->train(faces, labels);
    return emotion_classifier;
}

bool stop = false;

void sigIntHandler(int signal) {
    stop = true;
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, sigIntHandler);
    CascadeClassifier face_classifier;
    Ptr<FaceRecognizer> emotion_recognizer;

    String face_cascade_name = "haarcascade_frontalface_default.xml";
    String window_name = "Emotion detection";
    Mat frame;

    if (!face_classifier.load(cascade_dir_path + face_cascade_name)) {
        printf("couldt load haar cascade data\n");
        return 1;
    }

    try {
        printf("loading emotion classifier");
        emotion_recognizer = trainEmotionClassifier(face_classifier);
        emotion_recognizer->save("er_eigenface.yml");
    } catch (cv::Exception& e) {
        cerr << "Error opening file. Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }


    VideoCapture capture(0);
    if (!capture.isOpened()) {
        printf("couldt load cam\n");
        waitKey();
        return 1;
    }

    while(!stop) {
        capture >> frame;
        if (frame.empty()) { cout << "Empty frame!" << endl; break; }

        detectFaces(face_classifier, emotion_recognizer, frame);
        imshow(window_name, frame);

        if (waitKey(30) == 27)
        {
            cout << "esc key is pressed by user" << endl;
            destroyWindow(window_name);
            destroyAllWindows();
            break;
        }
    }
    capture.release();
    waitKey(0);
    return 0;
}

