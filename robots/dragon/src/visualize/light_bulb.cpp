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
    marker_array.markers.resize(3);

    visualization_msgs::Marker marker;
    marker.header.frame_id = "dragon/root";
    marker.header.stamp = ros::Time::now();
    marker.ns = "basic_shapes";
    marker.type = visualization_msgs::Marker::CUBE;
    marker.action = visualization_msgs::Marker::ADD;
    marker.lifetime = ros::Duration();
    marker.pose.orientation.x = -0.0740883274;
    marker.pose.orientation.y = 0.1490180347;
    marker.pose.orientation.z = 0.0111973555;
    marker.pose.orientation.w = 0.9859914626;
    marker.color.r = 0.392f;
    marker.color.g = 0.251f;
    marker.color.b = 0.204f;
    marker.color.a = 1.0f;

    marker.id = 0;
    marker.scale.x = 0.25;
    marker.scale.y = 0.05;
    marker.scale.z = 2.0;
    marker.pose.position.x = 0.4;
    marker.pose.position.y = 0.2;
    marker.pose.position.z = 0.7;
    marker_array.markers[0] = marker;
    
    marker.id = 1;
    marker.scale.x = 0.05;
    marker.scale.y = 0.25;
    marker.scale.z = 2.0;
    marker.pose.position.x = 0.5;
    marker.pose.position.y = 0.2;
    marker.pose.position.z = 0.7-0.0308;
    marker_array.markers[1] = marker;

    marker.id = 2;
    marker.scale.x = 0.05;
    marker.scale.y = 0.25;
    marker.scale.z = 2.0;
    marker.pose.position.x = 0.3;
    marker.pose.position.y = 0.2;
    marker.pose.position.z = 0.7+0.0308;
    marker_array.markers[2] = marker;


    marker_pub.publish(marker_array);

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}