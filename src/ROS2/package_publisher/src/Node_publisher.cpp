#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <ament_index_cpp/get_package_share_directory.hpp>

class Publisher: public rclcpp::Node
{
    private:
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr string_pub;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub;
        rclcpp::TimerBase::SharedPtr timer_;  

        std_msgs::msg::String str;
        cv::Mat img;
        
        void timer_callback() 
        {
            str.data = "势如金牛，一瞥惊鸿";
            string_pub->publish(str);
            RCLCPP_INFO(this->get_logger(), "str发送成功");

            sensor_msgs::msg::Image::SharedPtr img_ptr = 
                cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", img).toImageMsg();
            image_pub->publish(*img_ptr);
            RCLCPP_INFO(this->get_logger(), "img发送成功");
        }

    public:
        Publisher():Node("Taurus_pub")
        {
            string_pub = this->create_publisher<std_msgs::msg::String>("Taurus_string", 10);
            image_pub = this->create_publisher<sensor_msgs::msg::Image>("Taurus_image", 10);

            std::string pkg_path = ament_index_cpp::get_package_share_directory("package_publisher");
            std::string image_path = pkg_path + "/image/image.png";
            img = cv::imread(image_path);

            if(img.empty())
            {
                RCLCPP_ERROR(this->get_logger(), "img读取失败");
            }
            
            // 创建定时器，每2秒发布一次
            timer_ = this->create_wall_timer(
                std::chrono::seconds(2),
                std::bind(&Publisher::timer_callback, this));
            
            RCLCPP_INFO(this->get_logger(), "发布者已启动，每2秒发布一次");
        }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Publisher>();
    rclcpp::spin(node); 
    rclcpp::shutdown();
    return 0;
}