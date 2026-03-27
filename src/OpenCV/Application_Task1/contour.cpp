#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{
	Mat image, hsv, mask1, mask2, mask;
	image = imread("apple.png");
	if (image.empty())
	{
		cout << "图片为空" << endl;
		return -1;
	}
	//上个程序得到的HSV范围
	int Hmin1 = 0, Hmax1 = 21, Hmin2 = 165, Hmax2 = 179;
	int Smin = 161, Smax = 255, Vmin = 90, Vmax = 255;

	//将图像从BGR转化为HSV
	cvtColor(image, hsv, COLOR_BGR2HSV);

	//获得掩码图像
	inRange(hsv, Scalar(Hmin1, Smin, Vmin), Scalar(Hmax1, Smax, Vmax), mask1);
	inRange(hsv, Scalar(Hmin2, Smin, Vmin), Scalar(Hmax2, Smax, Vmax), mask2);
	mask = mask1 + mask2;

	//形态学操作，除去一些噪点和空洞
	Mat temp1, temp2;
	Mat kernel = getStructuringElement(MORPH_RECT, Size(7, 7));
	morphologyEx(mask, temp1, MORPH_CLOSE, kernel);
	morphologyEx(temp1, temp2, MORPH_OPEN, kernel);
	mask = temp2;

	//寻找轮廓
	vector<vector<Point>> contours;
	vector<Vec4i> hirearchy;
	findContours(mask, contours,hirearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    //遍历找出最大轮廓
	int max_area = 0;
	int max_id = -1;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contourArea(contours[i]) > max_area)
		{
			max_area = contourArea(contours[i]);
			max_id = i;
		}
	}

    //画出轮廓
	drawContours(image, contours, max_id, Scalar(0, 255, 0), 2);
	Rect rect = boundingRect(contours[max_id]);
	rectangle(image, rect, Scalar(255, 0, 0), 2);

	
    //展示图片
	namedWindow("apple_image", WINDOW_NORMAL);
	resizeWindow("apple_image", 1000, 600);
	imshow("apple_image", image);

	waitKey(0)
	return 0;
}