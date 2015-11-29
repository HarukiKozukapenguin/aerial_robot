#ifndef LASER_2DSLAM_H
#define LASER_2DSLAM_H

//* ros
#include <ros/ros.h>
#include <aerial_robot_base/basic_state_estimation.h>
#include <aerial_robot_base/kalman_filter.h>
#include <aerial_robot_base/digital_filter.h>
#include <tf/transform_broadcaster.h>

#include <aerial_robot_base/States.h>
#include <geometry_msgs/PoseStamped.h>

class SlamData
{
 public:
 SlamData(ros::NodeHandle nh,
          ros::NodeHandle nh_private,
          BasicEstimator* state_estimator,
          bool kalman_filter_flag,
          KalmanFilterPosVelAcc *kf_x, 
          KalmanFilterPosVelAcc *kf_y,
          KalmanFilterPosVelAccBias *kfb_x, 
          KalmanFilterPosVelAccBias *kfb_y,
          bool kalman_filter_debug,
          int kalman_filter_axis,
          KalmanFilterPosVelAccBias *kf1,
          KalmanFilterPosVelAccBias *kf2)
   : nh_(nh, "2dslam"), nhp_(nh_private, "2dslam")
      {
        rosParamInit(nhp_);

        slam_pub_ = nh_.advertise<aerial_robot_base::States>("state", 10);
        slam_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>("slam_out_pose", 5, boost::bind(&SlamData::poseStampedCallback, this, _1, state_estimator));

        pos_x_ = 0; pos_y_ = 0; psi_ = 0; 
        raw_pos_x_ = 0; raw_pos_y_ = 0; raw_psi_ = 0;
        vel_x_ = 0; vel_y_ = 0; vel_psi_ = 0; 
        raw_vel_x_ = 0;raw_vel_y_ = 0; raw_vel_psi_ = 0;

        filter_x_ =     IirFilter((float)rx_freq_x_, 
                                  (float)cutoff_pos_freq_x_, 
                                  (float)cutoff_vel_freq_x_);

        filter_y_ =     IirFilter((float)rx_freq_y_, 
                                 (float)cutoff_pos_freq_y_, 
                                 (float)cutoff_vel_freq_y_);

        filter_psi_ =     IirFilter((float)rx_freq_psi_, 
                                   (float)cutoff_pos_freq_psi_, 
                                   (float)cutoff_vel_freq_psi_);


        kalman_filter_flag_ = kalman_filter_flag;
        kalman_filter_debug_ = kalman_filter_debug;
        kalman_filter_axis_ = kalman_filter_axis;

        kf_x_ = kf_x; kf_y_ = kf_y; 
        kfb_x_ = kfb_x; kfb_y_ = kfb_y; 
        kf1_ = kf1; kf2_ = kf2; 

      }
  ~SlamData()
    {
    }

  static const int X = 0;
  static const int Y = 1;
  static const int YAW = 2;


  inline void setPosXValue(float pos_x_value)  {    pos_x_ = pos_x_value;  }
  inline void setPosYValue(float pos_y_value)  {    pos_y_ = pos_y_value;  }
  inline void setPsiValue(float psi_value)  {    psi_ = psi_value;  }
  inline float getPosXValue()  {    return  pos_x_;  }
  inline float getPosYValue()  {    return pos_y_;   }
  inline float getPsiValue()  {    return psi_;  }

  inline void setRawPosXValue(float raw_pos_x_value) { raw_pos_x_ = raw_pos_x_value;  }
  inline void setRawPosYValue(float raw_pos_y_value)  {    raw_pos_y_ = raw_pos_y_value;  }
  inline void setRawPsiValue(float raw_psi_value)  {    raw_psi_ = raw_psi_value;  }
  inline float getRawPosXValue()  {    return raw_pos_x_;  }
  inline float getRawPosYValue()  {    return raw_pos_y_;  }
  inline float getRawPsiValue()  {    return raw_psi_;  }

  inline void setVelXValue(float vel_x_value)  {    vel_x_ = vel_x_value;  }  
  inline void setVelYValue(float vel_y_value)  {    vel_y_ = vel_y_value;  }
  inline void setVelPsiValue(float vel_psi_value)  {    vel_psi_ = vel_psi_value;  }
  float getVelXValue()  {    return vel_x_;  }
  float getVelYValue()  {    return vel_y_;  }
  float getVelPsiValue()  {    return vel_psi_;  }

  inline void setRawVelXValue(float raw_vel_x_value)  {    raw_vel_x_ = raw_vel_x_value;  }
  inline void setRawVelYValue(float raw_vel_y_value)  {    raw_vel_y_ = raw_vel_y_value;  }
  inline void setRawVelPsiValue(float raw_vel_psi_value)  {    raw_vel_psi_ = raw_vel_psi_value;  }
  inline float getRawVelXValue()  {    return raw_vel_x_;  }
  inline float getRawVelYValue()  {    return raw_vel_y_;  }
  inline float getRawVelPsiValue()  {    return raw_vel_psi_;  }


 private:
  ros::NodeHandle nh_;
  ros::NodeHandle nhp_;
  ros::Publisher slam_pub_;
  ros::Subscriber slam_sub_;
  ros::Time stamp_;


  bool kalman_filter_flag_;
  KalmanFilterPosVelAcc *kf_x_;
  KalmanFilterPosVelAcc *kf_y_;

  KalmanFilterPosVelAccBias *kfb_x_;
  KalmanFilterPosVelAccBias *kfb_y_;

  bool kalman_filter_debug_;
  int  kalman_filter_axis_;
  KalmanFilterPosVelAccBias *kf1_;
  KalmanFilterPosVelAccBias *kf2_;


  double pos_x_;
  double pos_y_;
  double psi_; //yaw angle
  
  double raw_pos_x_;
  double raw_pos_y_;
  double raw_psi_;

  double vel_x_;
  double vel_y_;
  double vel_psi_; 

  double raw_vel_x_;
  double raw_vel_y_;
  double raw_vel_psi_;
  //+*+*+*+ filter types

  double rx_freq_x_;
  double rx_freq_y_;
  double rx_freq_psi_;
  double cutoff_pos_freq_x_;
  double cutoff_pos_freq_y_;
  double cutoff_pos_freq_psi_;
  double cutoff_vel_freq_x_;
  double cutoff_vel_freq_y_;
  double cutoff_vel_freq_psi_;


  IirFilter filter_x_;
  IirFilter filter_y_;
  IirFilter filter_psi_;

  void rosParamInit(ros::NodeHandle nh)
  {
    std::string ns = nh.getNamespace();
    if (!nh.getParam ("rx_freq_x", rx_freq_x_))
      rx_freq_x_ = 0;
    printf("%s: rx_freq_x_ is %.3f\n", ns.c_str(), rx_freq_x_);

    if (!nh.getParam ("rx_freq_y", rx_freq_y_))
      rx_freq_y_ = 0;
    printf("%s: rx_freq_y_ is %.3f\n", ns.c_str(), rx_freq_y_);

    if (!nh.getParam ("rx_freq_psi", rx_freq_psi_))
      rx_freq_psi_ = 0;
    printf("%s: rx_freq_psi_ is %.3f\n", ns.c_str(), rx_freq_psi_);

    if (!nh.getParam ("cutoff_pos_freq_x", cutoff_pos_freq_x_))
      cutoff_pos_freq_x_ = 0;
    printf("%s: cutoff_pos_freq_x_ is %.3f\n", ns.c_str(), cutoff_pos_freq_x_);

    if (!nh.getParam ("cutoff_pos_freq_y", cutoff_pos_freq_y_))
      cutoff_pos_freq_y_ = 0;
    printf("%s: cutoff_pos_freq_y_ is %.3f\n", ns.c_str(), cutoff_pos_freq_y_);

    if (!nh.getParam ("cutoff_pos_freq_psi", cutoff_pos_freq_psi_))
      cutoff_pos_freq_psi_ = 0;
    printf("%s: cutoff_pos_freq_psi_ is %.3f\n", ns.c_str(), cutoff_pos_freq_psi_);

    if (!nh.getParam ("cutoff_vel_freq_x", cutoff_vel_freq_x_))
      cutoff_vel_freq_x_ = 0;
    printf("%s: cutoff_vel_freq_x_ is %.3f\n", ns.c_str(), cutoff_vel_freq_x_);

    if (!nh.getParam ("cutoff_vel_freq_y", cutoff_vel_freq_y_))
      cutoff_vel_freq_y_ = 0;
    printf("%s: cutoff_vel_freq_y_ is %.3f\n", ns.c_str(), cutoff_vel_freq_y_);

    if (!nh.getParam ("cutoff_vel_freq_psi", cutoff_vel_freq_psi_))
      cutoff_vel_freq_psi_ = 0;
    printf("%s: cutoff_vel_freq_psi_ is %.3f\n", ns.c_str(), cutoff_vel_freq_psi_);

  }

  void poseStampedCallback(const geometry_msgs::PoseStampedConstPtr & pose_msg,
                           BasicEstimator* state_estimator)
  {
    static bool first_flag = true;    
    static float prev_raw_pos_x, prev_raw_pos_y, prev_raw_pos_psi;
    static double previous_secs;
    double current_secs = pose_msg->header.stamp.toSec();

    if(first_flag)
      {
        prev_raw_pos_x = 0; prev_raw_pos_y = 0; prev_raw_pos_psi = 0;
        first_flag = false;
       
        kfb_x_->setMeasureFlag();
        kfb_y_->setMeasureFlag();
       
        kf1_->setMeasureFlag();
        kf2_->setMeasureFlag();
       
        kf_x_->setMeasureFlag();
        kf_y_->setMeasureFlag();
      }
    else
      {
        //**** 位置情報の更新
        raw_pos_x_ = pose_msg->pose.position.x; 
        raw_pos_y_ = pose_msg->pose.position.y;    
        raw_psi_ =  tf::getYaw(pose_msg->pose.orientation); 
        raw_vel_x_ = (raw_pos_x_ - prev_raw_pos_x) / (current_secs - previous_secs);
        raw_vel_y_ = (raw_pos_y_ - prev_raw_pos_y) / (current_secs - previous_secs);

        if(raw_psi_ - prev_raw_pos_psi > M_PI)
          raw_vel_psi_ = (- 2 * M_PI + raw_psi_ - prev_raw_pos_psi)
            / (current_secs - previous_secs);
        else if(raw_psi_ - prev_raw_pos_psi < -M_PI)
          raw_vel_psi_ = (2 * M_PI + raw_psi_ - prev_raw_pos_psi)
            / (current_secs - previous_secs);
        else
          raw_vel_psi_ = (raw_psi_ - prev_raw_pos_psi)
            / (current_secs - previous_secs);


        //IIR filter
        filter_x_.filterFunction(raw_pos_x_, pos_x_, raw_vel_x_, vel_x_);
        filter_y_.filterFunction(raw_pos_y_, pos_y_, raw_vel_y_, vel_y_);
        filter_psi_.filterFunction(raw_psi_, psi_, raw_vel_psi_, vel_psi_);

        if(kalman_filter_flag_)
          {

            kf_x_->imuQuCorrection(pose_msg->header.stamp, raw_pos_x_);
            kf_y_->imuQuCorrection(pose_msg->header.stamp, raw_pos_y_);
            kfb_x_->imuQuCorrection(pose_msg->header.stamp, raw_pos_x_);
            kfb_y_->imuQuCorrection(pose_msg->header.stamp, raw_pos_y_);
          }

        if(kalman_filter_debug_)
          {
            if(kalman_filter_axis_ == X) 
              {
                kf1_->correction(raw_pos_x_,pose_msg->header.stamp);
                kf2_->correction(raw_pos_x_,pose_msg->header.stamp);

              }
            else if(kalman_filter_axis_ == Y) 
              {
                kf1_->correction(raw_pos_y_,pose_msg->header.stamp);
                kf2_->correction(raw_pos_y_,pose_msg->header.stamp);

              }
            else{}
          }

        aerial_robot_base::States three_axis_state;
        three_axis_state.header.stamp = pose_msg->header.stamp;

        aerial_robot_base::State x_state;
        x_state.id = "x";
        x_state.pos = pos_x_;
        x_state.raw_pos = raw_pos_x_;
        x_state.vel = vel_x_;
        x_state.raw_vel = raw_vel_x_;


        aerial_robot_base::State y_state;
        y_state.id = "y";
        y_state.pos = pos_y_;
        y_state.raw_pos = raw_pos_y_;
        y_state.vel = vel_y_;
        y_state.raw_vel = raw_vel_y_;

        aerial_robot_base::State yaw_state;
        yaw_state.id = "yaw";
        yaw_state.pos = psi_;
        yaw_state.raw_pos = raw_psi_;
        yaw_state.vel = vel_psi_;
        yaw_state.raw_vel = raw_vel_psi_;


        if(kalman_filter_flag_)
          {
            Eigen::Vector2d kf_x_state = kf_x_->getEstimateState();
            Eigen::Vector2d kf_y_state = kf_y_->getEstimateState();
            Eigen::Vector3d kfb_x_state = kfb_x_->getEstimateState();
            Eigen::Vector3d kfb_y_state = kfb_y_->getEstimateState();

            x_state.kf_pos = kf_x_state(0);
            x_state.kf_vel = kf_x_state(1);
            x_state.kfb_pos = kfb_x_state(0);
            x_state.kfb_vel = kfb_x_state(1);
            x_state.kfb_bias = kfb_x_state(2);

            y_state.kf_pos = kf_y_state(0);
            y_state.kf_vel = kf_y_state(1);
            y_state.kfb_pos = kfb_y_state(0);
            y_state.kfb_vel = kfb_y_state(1);
            y_state.kfb_bias = kfb_y_state(2);

          }

        three_axis_state.states.push_back(x_state);
        three_axis_state.states.push_back(y_state);
        three_axis_state.states.push_back(yaw_state);


        if(kalman_filter_debug_)
          {

            Eigen::Vector3d kf1_state = kf1_->getEstimateState();
            Eigen::Vector3d kf2_state = kf2_->getEstimateState();

            aerial_robot_base::State debug1;
            debug1.id = "debug1";
            debug1.pos =  kf1_state(0);
            debug1.vel =  kf1_state(1);

            aerial_robot_base::State debug2;
            debug2.id = "debug2";
            debug2.pos = kf2_state(0);
            debug2.vel = kf2_state(1);

            three_axis_state.states.push_back(debug1);
            three_axis_state.states.push_back(debug2);
          }

        slam_pub_.publish(three_axis_state);
      }

    //更新
    previous_secs = current_secs;
    prev_raw_pos_x = raw_pos_x_;
    prev_raw_pos_y = raw_pos_y_;
    prev_raw_pos_psi =  tf::getYaw(pose_msg->pose.orientation);
  }

};

#endif





