#include <opencv2/opencv.hpp>
#include<iostream>

using namespace std;
using namespace cv;

Mat img, temp;
bool is_draw = false;
Rect box;
//定义鼠标回调函数
void Mouce(int event,int x,int y,int flags , void*)
{
	switch (event)
	{
	//左键点击
	case EVENT_LBUTTONDOWN:
		is_draw = true;
		box = Rect(x, y, 0, 0);
		img.copyTo(temp); 
		break;
    //鼠标滑动
	case EVENT_MOUSEMOVE:
		//判断左键是否处于按住状态
		if (is_draw)
		{
			box.width = x - box.x;
			box.height = y - box.y;
			//将img还原为最初的img，防止在画过矩形框的img再次画上矩形框
			temp.copyTo(img);
			rectangle(img, box, Scalar(0, 255, 0), 2);
			//像素值
			Vec3b pixel = temp.at<Vec3b>(y, x);
			string text = format("(%d,%d) B:%d G:%d R:%d", x, y, pixel[0], pixel[1], pixel[2]);
			putText(img, text, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
			imshow("cat_image", img);
		}
		break;

	case EVENT_LBUTTONUP:
		//左键松开
		is_draw = false;
		if (box.width > 0 && box.height > 0)
		{
			//用得到的矩形框中裁剪img
			Mat roi = temp(box);
			imwrite("cat_roi.png", roi);
			namedWindow("cat_roi", WINDOW_NORMAL);
			resizeWindow("cat_roi", 600, 450);
			imshow("cat_roi", roi);
			//输出中心坐标
			Point center(box.x + box.width / 2, box.y + box.height / 2);
			cout << "Center Point : " << "(" << center.x << "," << center.y << ")" << endl;
		}
		break;
	}
}

int main()
{
	img = imread("cat.png");
	if (img.empty())
	{
		cout << "图片为空" << endl;
		return -1;
	}
	namedWindow("cat_image", WINDOW_NORMAL);
	resizeWindow("cat_image", 1200, 900);
	
	setMouseCallback("cat_image", Mouce, nullptr);

	imshow("cat_image", img);
	waitKey(0);
	return 0;
}
