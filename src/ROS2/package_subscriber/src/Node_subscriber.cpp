#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class Subscriber: public rclcpp::Node
{
private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr string_sub;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub;
    cv::Mat cached_img;

    void string_callback(const std_msgs::msg::String::SharedPtr str)
    {
        RCLCPP_INFO(this->get_logger(), "str已收到: %s", str->data.c_str());
    }

    void image_callback(const sensor_msgs::msg::Image::SharedPtr Img)
    {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(Img);
            cached_img = cv_ptr->image.clone();
            RCLCPP_INFO(this->get_logger(), "img读取成功,尺寸: %dx%d", cached_img.cols, cached_img.rows);
        
    }

    void display()
    {
        cv::namedWindow("图像窗口", cv::WINDOW_NORMAL);
        while (rclcpp::ok()) {
            {
                if (!cached_img.empty()) {
                    cv::imshow("图像窗口", cached_img);
                }
            }
            cv::waitKey(30);
            rclcpp::spin_some(this->shared_from_this());
        }
        cv::destroyAllWindows();
    }

public:
    Subscriber(): Node("Taurus_sub")
    {
        string_sub = this->create_subscription<std_msgs::msg::String>(
            "Taurus_string", 10,
            std::bind(&Subscriber::string_callback, this, std::placeholders::_1));
        image_sub = this->create_subscription<sensor_msgs::msg::Image>(
            "Taurus_image", 10,
            std::bind(&Subscriber::image_callback, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "订阅者已启动");
    }
    void run()
    {
        display();
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Subscriber>();
    node->run();
    rclcpp::shutdown();
    return 0;
}
//程序在运行前需要执行 export LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0
//强制程序运行时优先加载系统指定路径下的 pthread 线程库