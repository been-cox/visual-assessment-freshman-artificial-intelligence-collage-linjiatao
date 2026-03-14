#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;
//定义部分全局变量
VideoCapture cap;
int w, h;
double fps;
int brightness = 100;
int exposure = -5;
//定义亮度调节函数
void Brightness_callback(int,void*)
{
	cap.set(CAP_PROP_BRIGHTNESS, brightness);
}
//定义曝光调节函数
void Exposure_callback(int, void*)
{
	cap.set(CAP_PROP_EXPOSURE, exposure);
}

int main()
{
	cap.open(0);

	//获取图像尺寸和帧率
	w = cap.get(CAP_PROP_FRAME_WIDTH);
	h = cap.get(CAP_PROP_FRAME_HEIGHT);
	fps = cap.get(CAP_PROP_FPS);

	//创建窗口
	namedWindow("滑动条窗口", WINDOW_NORMAL);
	namedWindow("摄像头窗口", WINDOW_NORMAL);
	createTrackbar("亮度", "滑动条窗口", &brightness, 100, Brightness_callback);
	createTrackbar("曝光度", "滑动条窗口",& exposure, 0, Exposure_callback);
	setTrackbarMin("曝光度","滑动条窗口",-13);
	setTrackbarMax("曝光度", "滑动条窗口", -1);

	//创建VideoWriter对象并定义编码格式
	VideoWriter writer;
	string file_name = "video.avi";
	int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
	writer.open(file_name, fourcc, fps, Size(w, h));

	//录制视频
	Mat frame;
	bool is_record = false;
	cout << "开关录制请按 r" << endl;
	cout << "退出请按 esc" << endl;
	while (true)
	{
		cap >> frame;
		if (frame.empty())
		{
			break;
		}
		string frame_info = format("%dx%d %.1f fps", w, h, fps);
		putText(frame, frame_info, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 200, 0), 2);

		if (is_record)
		{
			//在frame上画上录制标识
			circle(frame, Point(w - 100, 50), 10, Scalar(0, 0, 255), -1);
			writer.write(frame);
		}
		imshow("摄像头窗口", frame);

		//设置按键功能
		char key = waitKey(1);
		if (key == 27)
		{
			break;
		}
		if (key == 'r')
		{
			is_record = !is_record;
		}
	}
	if (writer.isOpened())
		writer.release();
	return 0;
}