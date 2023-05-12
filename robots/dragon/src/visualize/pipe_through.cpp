#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include <string>
#include <math.h>
#include <eigen3/Eigen/Eigen>
template <int rows = Eigen::Dynamic>
using Vector = Eigen::Matrix<float, rows, 1>;

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
    marker_array.markers.resize(1);

    visualization_msgs::Marker marker;
    marker.header.frame_id = "dragon/link4";
    marker.header.stamp = ros::Time::now();
    marker.ns = "basic_shapes";
    marker.type = visualization_msgs::Marker::MESH_RESOURCE;
    marker.action = visualization_msgs::Marker::ADD;
    marker.lifetime = ros::Duration();
    float roll = 0.0, pitch = 0.0, yaw = 3.14;
    Eigen::Quaternion<float> q;
    q = Eigen::AngleAxis<float>(roll, Vector<3>::UnitX()) *
    Eigen::AngleAxis<float>(pitch, Vector<3>::UnitY()) *
    Eigen::AngleAxis<float>(yaw, Vector<3>::UnitZ());
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
    marker.pose.position.x = 0.65;
    marker.pose.position.y = 1.0;
    marker.pose.position.z = -0.15;

    marker.color.r = 0.5f;
    marker.color.g = 0.5f;
    marker.color.b = 0.5f;
    marker.color.a = 0.7f;
    marker.scale.x = 10.0;
    marker.scale.y = 10.0;
    marker.scale.z = 10.0;

    marker.id = 0;
    marker.mesh_resource = "package://dragon/urdf/mesh/pipe.dae";
    marker_array.markers[0] = marker;


    marker_pub.publish(marker_array);

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}