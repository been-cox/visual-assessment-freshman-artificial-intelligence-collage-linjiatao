//拍摄并保存棋盘格照片
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;
int main()
{
	//打开摄像头
	VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "无法打开摄像头" << endl;
		return -1;
	}

	Mat image;
	int num = 1;
	namedWindow("摄像头", WINDOW_NORMAL);
	//保存图片快捷键
	cout << "按r保存，按esc退出" << endl;

	while (true)
	{
		cap >> image;
		if (image.empty())
		{
			break;
		}
		imshow("摄像头", image);
		//获取快捷键
		char key = waitKey(1);
		if (key == 'r')
		{
			string image_name = format("image_%d.png",num);
			imwrite(image_name, image);
			cout << "已保存"<<num<<"张图片" << endl;
			num++;
		}
		//按Esc退出程序
		else if (key == 27)
		{
			break;
		}
	}
	return 0;
}