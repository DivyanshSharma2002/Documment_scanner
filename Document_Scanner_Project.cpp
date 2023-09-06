#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat img, imggryblr, imgcany;
Mat imgcontr;
Mat imgwarp;
Mat imgclr;
vector<Point> imgpoint;
vector<Point> newpoints;
float w = 420, h = 596;

void drawconts(vector<Point> points, Scalar color)
{
    for (int i = 0; i < points.size(); i++)
    {
        circle(img, points[i], 4, color, FILLED);
    }
    line(img, points[0], points[1], color, 2);
    line(img, points[1], points[3], color, 2);
    line(img, points[2], points[3], color, 2);
    line(img, points[0], points[2], color, 2);
}

Mat preprocess(Mat img)
{
    Mat imggryblr, imgcany;
    cvtColor(img, imggryblr, COLOR_BGR2GRAY);
    GaussianBlur(imggryblr, imggryblr, Size(5, 5), 5, 0);
    Canny(imggryblr, imgcany, 30, 100);
    Mat k = getStructuringElement(MORPH_RECT, Size(3, 3));
    dilate(imgcany, imgcany, k);
    return imgcany;
}

vector<Point> getContours(Mat imgcany, Mat img) {

    vector<vector<Point>> contours;
    vector<Vec4i> hirarchy;

    findContours(imgcany, contours, hirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> contpoly(contours.size());
    vector<Rect> brect(contours.size());

    vector<Point> bigest;
    int Maxarea = 0;

    for (int i = 0; i < contours.size(); i++)
    {
        int area = contourArea(contours[i]);
        if (area > 10000)
        {
            float peri = arcLength(contours[i], true);
            approxPolyDP(contours[i], contpoly[i], 0.02 * peri, true);
            if (contpoly[i].size() == 4 && area > Maxarea)
            {
                bigest = { contpoly[i][0],contpoly[i][1], contpoly[i][2], contpoly[i][3] };
                Maxarea = area;
            }

        }
    }
    return bigest;
}

vector<Point> reorder(vector<Point> repoint)
{
    vector<Point> newpoint;
    vector<int> sump, subp;

    for (int i = 0; i < 4; i++)
    {
        sump.push_back(repoint[i].x + repoint[i].y);
        subp.push_back(repoint[i].x - repoint[i].y);
    }

    newpoint.push_back(repoint[min_element(sump.begin(), sump.end()) - sump.begin()]);
    newpoint.push_back(repoint[max_element(subp.begin(), subp.end()) - subp.begin()]);
    newpoint.push_back(repoint[min_element(subp.begin(), subp.end()) - subp.begin()]);
    newpoint.push_back(repoint[max_element(sump.begin(), sump.end()) - sump.begin()]);

    return newpoint;
}

Mat warp(Mat img, vector<Point> point, float w, float h)
{
    Point2f scr[4] = { point[0],point[1], point[2], point[3] };
    Point2f dst[4] = { {0.0f,0.0f},{w,0.0f}, {0.0f,h}, {w,h} };

    Mat matrix = getPerspectiveTransform(scr, dst);
    Mat imgwarp;
    warpPerspective(img, imgwarp, matrix, Point(w, h));

    float crpv = 5;
    Rect roi(crpv, crpv, w - (2 * crpv), h - (2 * crpv));
    imgwarp = imgwarp(roi);

    return imgwarp;
}

int main() {

    Mat img, imgcany, imgwarp;
    vector<Point> imgpoint, newpoints;
    float w = 420, h = 596;

    /*int camerano = 0;
    cout << "Enter the camera ID" << endl;
    cout << "0-For Default camera " << endl;
    cout << "1-If Additional webcam is attached " << endl;
    cin >> camerano;*/
    /*   VideoCapture cap(0);*/
    string path = ("resource/he.jpg");

    /*if (!cap.isOpened()) {
        cerr << "Error: Unable to access the camera." << endl;
        return -1;
    }*/

    while (true)
    {
        try {
            /*  cap.read(img);*/
            img = imread(path);
            if (img.empty()) {
                cerr << "Error: Unable to read the frame from the camera." << endl;
                break;
            }
            flip(img, img, 1);

            imgcany = preprocess(img);

            imgpoint = getContours(imgcany, img);
            if (!imgpoint.empty())
            {
                newpoints = reorder(imgpoint);

                imgwarp = warp(img, newpoints, w, h);
                flip(imgwarp, imgwarp, 1);
                imshow("Orignal Image Points", img);
                imshow("Image warp", imgwarp);
                /*  imwrite("resource/2.png", imgwarp);*/
                drawconts(newpoints, Scalar(0, 255, 255));
            }

            /* imshow("Orignal Image Points", img);*/
             //if (waitKey(0) == 27) {
             //    break; // Press 'Esc' to exit the loop.
             //}

        }
        catch (const exception& ex) {
            cerr << "Exception: " << ex.what() << endl;
            break;
        }
    }
    waitKey(0);
    return 0;
}
