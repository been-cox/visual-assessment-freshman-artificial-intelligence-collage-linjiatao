#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main()
{
	string image_path ="image.png";
	Mat image = imread(image_path);
	if (image.empty())
	{
		cout << "图片为空" << endl;
	}
	//从BGR转化为HSV，并将红色作为目标颜色得到二值图
	Mat hsv,Binary,Binary1, Binary2;
	cvtColor(image, hsv, COLOR_BGR2HSV);
	inRange(hsv, Scalar(0, 100, 80), Scalar(10, 255, 255),Binary1);
	inRange(hsv, Scalar(165, 100, 80), Scalar(180, 255, 255),Binary2);
	Binary = Binary1 + Binary2;

	//形态学操作，除去噪点和空洞
	Mat temp1, temp2;
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(Binary, temp1, MORPH_OPEN, kernel);
	morphologyEx(temp1, temp2, MORPH_CLOSE, kernel);
	Binary = temp2;

	//Canny边缘检测
	Mat canny;
	Canny(Binary, canny, 50, 150);

	//将canny中的白色边缘变为蓝色边缘
	Mat  blue_edge;
	blue_edge = Mat::zeros(image.size(), image.type());
	blue_edge .setTo (Scalar(230, 191, 0), canny);

	//创建窗户并指定大小
	namedWindow("Original image", WINDOW_NORMAL);
	resizeWindow("Original image", 1280, 720);

	namedWindow("Binary image", WINDOW_NORMAL);
	resizeWindow("Binary image", 1280, 720);

	namedWindow("Edge image", WINDOW_NORMAL);
	resizeWindow("Edge image", 1280, 720);
	
	//显示图像
	imshow("Original image", image);
	imshow("Binary image", Binary);
	imshow("Edge image", blue_edge);

	//保存图像
	imwrite("canny_edge_image.png", blue_edge);

	waitKey(0);
	return 0;
}