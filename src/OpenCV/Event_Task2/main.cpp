#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <deque>

using namespace std;
using namespace cv;

int main()
{
	//读取视频
	VideoCapture cap("Rune_red.mp4");
	if (!cap.isOpened())
	{
		cout << "无法读取视频" << endl;
		return -1;
	}
	//上一个目标扇叶的矩形中心(跟踪目标扇叶）
	Point2f last_tune_center(-1, -1);

	//绘制角速度曲线的变量
	auto global_start_time = chrono::high_resolution_clock::now();
	double last_time = 0;//上一帧的绝对时间
	float last_angle = 0;//上一帧的角度
	deque<float> speeds;//记录角速度，用于画线
	int direction = 0;//1代表顺时针，-1代表逆时针
	float last_speed = 0;

	while (true)
	{
		//开始计时
		auto start_time = chrono::high_resolution_clock::now();//模块运行开始时间
		double current_t = chrono::duration<double>(start_time - global_start_time).count();//全局时间与当前时刻的时间差

		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout<< "视频播放完毕" << endl;
			break;
		}

		//图像预处理
		vector<Mat> channels;
		split(frame, channels);//得到三通道灰度图
		Mat gray_img, mask;
		subtract(channels[2], channels[0], gray_img);
		threshold(gray_img, mask, 80, 255, THRESH_BINARY);//得到目标颜色二值图

		Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
		morphologyEx(mask, mask, MORPH_CLOSE, kernel);

		//找轮廓
		vector<vector<Point>> contours;
		findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		//通过遍历轮廓找到目标扇叶并且收集中心点R
		RotatedRect Tune_dst;
		vector<Point2f> Rs;
		Point2f best_R;
		bool found_tune = false;
		bool found_R = false;
		float min_tune_dist = 1e9;

		for (size_t i = 0; i < contours.size(); i++)
		{
			//轮廓面积
			double area = contourArea(contours[i]);
			//过滤噪点
			if (area < 150)
				continue;
			//轮廓长宽比
			RotatedRect rect = minAreaRect(contours[i]);
			float w = rect.size.width;
			float h = rect.size.height;
			float ratio = max(w, h) / (min(w, h) + 0.001f);

			//通过面积限制和长宽比限制找到目标扇叶并收集R
			if (area > 3000 && ratio > 1.8)
			{
				if (last_tune_center.x != -1)
				{
					//优先选择和上一帧目标扇叶最近的扇叶
					float dist = norm(rect.center - last_tune_center);
					if (dist < min_tune_dist)
					{
						min_tune_dist = dist;
						Tune_dst = rect;
						found_tune = true;
					}
				}
				else
				{
					//首次识别，直接赋值
					Tune_dst = rect;
					found_tune = true;
				}
			}
			//R的收集
			else if (area > 150 && area < 3000 && ratio>0.8 && ratio < 1.5)
			{
				Rs.push_back(rect.center);
			}
		}

		//如果找到了目标扇叶，从收集的R中找到目标R
		Point2f R_dst;
		//这里采用的方法是通过筛选R到目标扇叶中心的矩形与目标扇叶长度的比值，同时限制R与目标扇叶的连线与目标扇叶的长的夹角
		if (found_tune && !Rs.empty())
		{
			Point2f rect_points[4];
			Tune_dst.points(rect_points);
			//计算扇叶长边的长度
			float d1 = norm(rect_points[0] - rect_points[1]);
			float d2 = norm(rect_points[1] - rect_points[2]);
			float tune_len = max(d1, d2);

			//找目标扇叶长轴所在的方向向量
			Point2f tune_vector;
			if (d1 > d2)
			{
				//如果0和1连成的线更长，说明目标向量在这条线上
				tune_vector = rect_points[0] - rect_points[1];
			}
			else
			{
				//如果1和2连成的线更长，说明目标向量在这条线上
				tune_vector = rect_points[1] - rect_points[2];
			}
			//该向量的长度
			float dist_vect_tune = norm(tune_vector);

			//计算该向量的单位向量
			Point2f vector01 = tune_vector / (dist_vect_tune + 0.001f);

			//后续筛选最佳比值
			float min_diff = 1e8;
			float best_ratio = 1.2;

			//开始遍历R，找出最符合要求的R
			for (size_t j = 0; j < Rs.size(); j++)
			{
				//R与目标扇叶中心连线的向量
				Point2f R_vector = Rs[j] - Tune_dst.center;
				//计算该向量距离
				float dist_vect_R = norm(R_vector);

				//R到扇叶圆心的距离与扇叶长度的比值
				float ratio = dist_vect_R / tune_len;

				//算出两个向量的单位向量
				Point2f vector02 = R_vector / (dist_vect_R+0.001f);

				//计算两个向量夹角的余弦值
				float cos = abs(vector01.dot(vector02));

				//将两向量夹角限制在16度内，并且限制R到扇叶圆心距离与扇叶长度的比值
				if (cos > 0.96 && ratio > 0.5 && ratio < 1.5)
				{
					float diff = abs(ratio - best_ratio);
					//将与best_ratio差异最小的R作为目标R
					if (diff < min_diff)
					{
						min_diff = diff;
						best_R = Rs[j];
						found_R = true;
					}
				}
			}
		}
		if (found_tune)
		{
			last_tune_center = Tune_dst.center;
		}
			
		//开始绘制
		//绘制目标扇叶
		if (found_tune)
		{
			Point2f tunepoints[4];
			Tune_dst.points(tunepoints);
			for (int i = 0; i < 4; i++)
			{
				line(frame, tunepoints[i], tunepoints[(i + 1) % 4], Scalar(0, 255, 255), 2);
			}
		}
		//绘制R的外圈
		if (found_R)
		{
			circle(frame, best_R, 13, Scalar(0, 255, 0), 2);
		}
		//绘制扇叶的装甲板中心和速度曲线
		if (found_tune && found_R)
		{
			Point2f tunepoints[4];
			Tune_dst.points(tunepoints);
			//找到扇叶离R最远的两个点
			vector<pair<float, Point2f>> dist_pts;
			for (int i = 0; i < 4; i++)
			{
				dist_pts.push_back({ norm(tunepoints[i] - best_R),tunepoints[i] });
			}
			sort(dist_pts.begin(), dist_pts.end(), [](const pair<float, Point2f>& a, const pair<float, Point2f>& b)
				{return a.first > b.first; });

			//获得扇叶上离R最远的两个点的中点
			Point2f p1 = dist_pts[0].second;
			Point2f p2 = dist_pts[1].second;
			Point2f midpoint = (p1 + p2) / 2;
			float width = norm(p2 - p1);

			//获得中点指向R的单位向量
			Point2f vector = best_R - midpoint;
			float length = norm(vector);
			Point2f unit_vector = vector / length;

			//获得扇叶上的装甲板中心并画出
			Point2f dstpoint = midpoint + unit_vector * (0.5f * width);
			circle(frame, dstpoint, 9, Scalar(255, 255, 0), -1);

			//计算当前角度（-3.14到3.14）
			float angle_now = atan2(dstpoint.y - best_R.y, dstpoint.x - best_R.x);
			//计算角速度
			if (last_time != 0)
			{
				double dt = current_t - last_time;
				float d_angle = angle_now - last_angle;
				//处理角度跳变
				if (d_angle > CV_PI)
				{
					d_angle -= 2 * CV_PI;
				}
				if (d_angle < -CV_PI)
				{
					d_angle += 2 * CV_PI;
				}
				//计算角速度
				if (abs(d_angle) < 0.3 && dt != 0)
				{
					float speed = d_angle / dt;
					//防像素抖动
					last_speed = 0.9 * last_speed + 0.1 * speed;
					//判断顺逆时针
					if (last_speed > 0)
						direction = 1;
					else
						direction = -1;
		
					speeds.push_back(abs(last_speed));
					if (speeds.size() > 800)
						speeds.pop_front();
				}
			}
			last_angle = angle_now;
			last_time = current_t;		
		}
		//开始绘制曲线
		Mat speedImg(300, 800, CV_8UC3, Scalar(255, 255, 255));
		for (size_t i = 1; i < speeds.size(); i++)
		{
			int y1 = 300-(int)((speeds[i - 1]/4)*200);
			int y2 = 300-(int)((speeds[i]/4)*200);
			Point p1(i - 1, y1);
			Point p2(i, y2);
			line(speedImg, p1, p2, Scalar(255, 0, 0), 1);
			circle(speedImg, p2, 2, Scalar(255, 0, 0), -1);
		}

		string speed;
		if (direction == 1)
			speed = format("Angle Speed: %.2f rad/s   Direction:clockwise", last_speed);
		else
			speed = format("Angle Speed: %.2f rad/s   Direction: counter clockwise", last_speed);

		putText(speedImg, speed, Point(25, 25), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 2);
		imshow("speedImg", speedImg);


		
		//模块耗时，FPS和中心点的打印输出
		auto end_time = chrono::high_resolution_clock::now();
		double delay_ms = chrono::duration<double, milli>(end_time -start_time ).count();
		int fps = (int)(1000.0 / delay_ms);

		cout << "Time per frame:" << delay_ms << "ms||FPS:" << fps << endl;
		if (found_R)
		{
			cout << "R_center" << "(" << (int)best_R.x << "," << (int)best_R.y << ")" << endl;
		}
		//展示效果
		imshow("Tune", frame);
		imshow("mask", mask);
		//按Esc退出
		if (waitKey(1) == 27)
			break;
	}
	cap.release();
	destroyAllWindows();
	return 0;
}