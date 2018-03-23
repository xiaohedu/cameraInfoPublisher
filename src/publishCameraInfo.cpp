//contains cameraInfo for mono camera.

#include <ros/ros.h>
#include <ros/package.h>
#include "sensor_msgs/CameraInfo.h"
#include "sensor_msgs/CompressedImage.h"
#include "sensor_msgs/Image.h"
#include "camera_info_manager/camera_info_manager.h"

using namespace std;

/**Code adapted from
http://answers.ros.org/question/59725/publishing-to-a-topic-via-subscriber-callback-function/
  More efficient code to modify image before republishing at
http://answers.ros.org/question/53234/processing-an-image-outside-the-callback-function/
*/
class publishCameraInfo
{
public:
  publishCameraInfo(const ros::NodeHandle &nh, const ros::NodeHandle &n): nh_(nh), n_(n)
  {
      string img_topic = "/camera_raw";

      if(n_.getParam("topic/img", img_topic))
          ROS_INFO("Get image info topic: %s", img_topic.c_str());
      else
          ROS_WARN("Use default image info topic: %s", img_topic.c_str());
      if(n_.getParam("topic/info", info_topic))
          ROS_INFO("Get image info topic: %s", info_topic.c_str());
      else
          ROS_WARN("Use default image info topic: %s", info_topic.c_str());

      pub_cam_info = nh_.advertise<sensor_msgs::CameraInfo>(info_topic, 1);

      sub_img = nh_.subscribe(img_topic, 1, &publishCameraInfo::callback, this);
  }
  void callback(const sensor_msgs::ImageConstPtr& imgmsg)
  {
    //  not matching camname to camera name in input file throws an error in camera_info_manager.
    //  This error can be ignored.
    //  const std::string camurl="file:///home/rnl/.ros/camera_info/snapcam1.yaml";
    std::string camname;

    n_.getParam ( "camfile", camname);  //camname is set by:  rosparam set camfile _____

    const std::string camnameConst = camname;

    const std::string camurlRead = "file://" + ros::package::getPath("undistort_images") + "/calib_files/" + camname + ".yaml";

    camera_info_manager::CameraInfoManager caminfo(nh_, camnameConst, camurlRead);

    sensor_msgs::CameraInfo ci;

    ci = caminfo.getCameraInfo();

    ci.header.stamp = imgmsg->header.stamp;
    ci.header.frame_id = imgmsg->header.frame_id;
    
    // Publish via image_transport
    pub_cam_info.publish(ci);
  }

  private:
  ros::NodeHandle n_,nh_;
  ros::Publisher pub_cam_info;
  ros::Subscriber sub_img;

  string info_topic = "/camera_info";

};//End of class

int main(int argc, char **argv)
{
    ros::init(argc, argv, "publishCameraInfo");
    ros::NodeHandle nh;
    ros::NodeHandle n("~");

    publishCameraInfo cameraPubObject(nh, n);

    ros::spin();

    return 0;
}



