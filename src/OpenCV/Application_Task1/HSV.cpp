//查找能画出苹果轮廓的HSV
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

//创建HSV变量
int Hmin1 = 0, Hmax1 = 22, Hmin2 = 165, Hmax2 = 179;
int Smin, Smax, Vmin, Vmax;

Mat image, hsv, mask , mask1 , mask2;
//创建滑动条回调函数
void trackbar(int, void*)
{
	//找出目标颜色为红色的二值图
	inRange(hsv, Scalar(Hmin1, Smin, Vmin), Scalar(Hmax1, Smax, Vmax), mask1);
	inRange(hsv, Scalar(Hmin2, Smin, Vmin), Scalar(Hmax2, Smax, Vmax), mask2);

	mask = mask1 + mask2;
	//打印S和V的信息
	cout << "Smin:" << Smin << "," << "Vmin:" << Vmin << endl;
	cout << "Smax:" << Smax << "," << "Vmax:" << Vmax << endl;

	imshow("图像窗口",mask);
}

int main()
{
	image = imread("apple.png");
	if (image.empty())
	{
		cout << "图片为空" << endl;
		return -1;
	}
	//将图片从BGR转化为HSV
	cvtColor(image, hsv, COLOR_BGR2HSV);

	namedWindow("滑动条窗口", WINDOW_NORMAL);
	namedWindow("图像窗口", WINDOW_NORMAL);
	resizeWindow("图像窗口", 1000, 600);

	//创建滑动条
	createTrackbar("Smax", "滑动条窗口", &Smax, 255, trackbar);
	createTrackbar("Smin", "滑动条窗口", &Smin, 255, trackbar);
	createTrackbar("Vmax", "滑动条窗口", &Vmax, 255, trackbar);
	createTrackbar("Vmin", "滑动条窗口", &Vmin, 255, trackbar);


	imshow("图像窗口", image);
	waitKey(0);
	return 0;
}