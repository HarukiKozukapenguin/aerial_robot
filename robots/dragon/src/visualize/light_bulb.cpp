#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include <string>
#include <math.h>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "steel_object_node");
  ros::NodeHandle nh;

  // publisher
  ros::Publisher marker_pub = nh.advertise<visualization_msgs::MarkerArray>("marker_array", 1);

  ros::Rate loop_rate(10);
  while (ros::ok())
  {
    visualization_msgs::MarkerArray marker_array;
    marker_array.markers.resize(2);

    visualization_msgs::Marker marker;
    marker.header.frame_id = "dragon/root";
    marker.header.stamp = ros::Time::now();
    marker.ns = "basic_shapes";
    marker.action = visualization_msgs::Marker::ADD;
    marker.lifetime = ros::Duration();
    marker.pose.orientation.x = -0.1947091712;
    marker.pose.orientation.y = 0.1947091712;
    marker.pose.orientation.z = 0.0394695030;
    marker.pose.orientation.w = 0.9605304970;
    marker.color.a = 1.0f;

    marker.id = 0;
    marker.type = visualization_msgs::Marker::CYLINDER;
    marker.scale.x = 0.2;
    marker.scale.y = 0.2;
    marker.scale.z = 0.3;
    marker.pose.position.x = 0.6;
    marker.pose.position.y = 0.3;
    marker.pose.position.z = 0.9;
    marker.color.r = 1.0f;
    marker.color.g = 1.0f;
    marker.color.b = 1.0f;
    marker_array.markers[0] = marker;
    
    marker.id = 1;
    marker.type = visualization_msgs::Marker::SPHERE;
    marker.scale.x = 0.6;
    marker.scale.y = 0.6;
    marker.scale.z = 0.6;
    marker.pose.position.x = 0.5;
    marker.pose.position.y = 0.2;
    marker.pose.position.z = 0.7-0.0308;
    marker.color.r = 1.0f;
    marker.color.g = 1.0f;
    marker.color.b = 0.50f;
    marker_array.markers[1] = marker;

    marker_pub.publish(marker_array);

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}