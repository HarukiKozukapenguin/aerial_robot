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
  float transition_pos_x = 0.0, transition_pos_y = -0.15, transition_pos_z = -0.13;
  float transition_ori_roll = 0., transition_ori_pitch = 0.0, transition_ori_yaw = 0.0;

  Eigen::Quaternion<float>  transition_q = Eigen::AngleAxis<float>(transition_ori_roll, Vector<3>::UnitX()) *
  Eigen::AngleAxis<float>(transition_ori_pitch, Vector<3>::UnitY()) *
  Eigen::AngleAxis<float>(transition_ori_yaw, Vector<3>::UnitZ());

  while (ros::ok())
  {
    visualization_msgs::MarkerArray marker_array;
    marker_array.markers.resize(3);

    visualization_msgs::Marker marker;
    marker.header.frame_id = "dragon/link6";
    marker.header.stamp = ros::Time::now();
    marker.ns = "basic_shapes";
    marker.action = visualization_msgs::Marker::ADD;
    marker.lifetime = ros::Duration();
    float roll = 0.0, pitch = 0.0, yaw = 0.0;
    Eigen::Quaternion<float> q;
    Vector<3> color;


    marker.id = 0;
    marker.type = visualization_msgs::Marker::CYLINDER;
    roll = 1.57, pitch = 0.0, yaw = 0.0;
    q = Eigen::AngleAxis<float>(roll, Vector<3>::UnitX()) *
    Eigen::AngleAxis<float>(pitch, Vector<3>::UnitY()) *
    Eigen::AngleAxis<float>(yaw, Vector<3>::UnitZ())*transition_q;
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
    marker.pose.position.x = 0.0+transition_pos_x;
    marker.pose.position.y = 0.2+transition_pos_y;
    marker.pose.position.z = .0+transition_pos_z;
    marker.scale.x = 0.12;
    marker.scale.y = 0.12;
    marker.scale.z = .6;
    color << 190, 193, 195;
    marker.color.r = color[0]/255;
    marker.color.g = color[1]/255;
    marker.color.b = color[2]/255;
    marker.color.a = 1.0f;
    marker_array.markers[0] = marker;

    marker.id = 1;
    marker.type = visualization_msgs::Marker::CYLINDER;
    roll = 0.0, pitch = 1.57, yaw = 0.0;
    q = Eigen::AngleAxis<float>(roll, Vector<3>::UnitX()) *
    Eigen::AngleAxis<float>(pitch, Vector<3>::UnitY()) *
    Eigen::AngleAxis<float>(yaw, Vector<3>::UnitZ())*transition_q;
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
    marker.pose.position.x = 0.3+transition_pos_x;
    marker.pose.position.y = -0.05+transition_pos_y;
    marker.pose.position.z = 0+transition_pos_z;
    marker.scale.x = 0.1;
    marker.scale.y = 0.1;
    marker.scale.z = .6;
    marker_array.markers[1] = marker;
    

    marker.id = 2;
    marker.type = visualization_msgs::Marker::CUBE;
    roll = 0, pitch = 0, yaw = 0.0;
    q = Eigen::AngleAxis<float>(roll, Vector<3>::UnitX()) *
    Eigen::AngleAxis<float>(pitch, Vector<3>::UnitY()) *
    Eigen::AngleAxis<float>(yaw, Vector<3>::UnitZ())*transition_q;
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
    marker.pose.position.x = .5;
    marker.pose.position.y = .6;
    marker.pose.position.z = -0.2+transition_pos_z;
    marker.scale.x = .2;
    marker.scale.y = 2.0;
    marker.scale.z = 4.0;
    color << 116, 80, 48;
    marker.color.r = color[0]/255;
    marker.color.g = color[1]/255;
    marker.color.b = color[2]/255;
    marker.color.a = 1.0f;
    marker_array.markers[2] = marker;


    marker_pub.publish(marker_array);

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}