#include <common_observation_cal.h>

ObstacleCalculator::ObstacleCalculator(ros::NodeHandle nh, ros::NodeHandle pnh)
    : nh_(nh), pnh_(pnh), call_(0) {

  std::string file, quad_name;
  pnh_.getParam("cfg_path", file);
  pnh_.getParam("hokuyo", from_hokuyo_);
  pnh_.getParam("shift_x", shift_x_);
  pnh_.getParam("shift_y", shift_y_);
  pnh_.getParam("robot_ns", quad_name);
  pnh_.getParam("print_yaw", print_yaw_);
  pnh_.getParam("vel_calc_boundary", vel_calc_boundary_);
  pnh_.getParam("body_r", body_r_);

  //   file = file + ".csv";
  if (!from_hokuyo_){
    std::cout << "file name is " << file << std::endl;
    std::ifstream ifs(file);
    if (!ifs) {
      std::cerr << "cannot open file" << std::endl;
      std::exit(1);
    }
    std::string line;

    while (std::getline(ifs, line)) {
      std::vector<std::string> strvec = split(line, ',');
      Eigen::Vector3d tree_pos;
      tree_pos(0) = stof(strvec.at(1))+shift_x_; //world coodinate
      // std::cout << "shift_x is: " << shift_x_ << std::endl;
      // std::cout << "tree_pos(0) is: " << tree_pos(0) << std::endl;
      tree_pos(1) = stof(strvec.at(2))+shift_y_; //world coodinate
      tree_pos(2) = 0;
      positions_.push_back(tree_pos);
      radius_list_.push_back(stof(strvec.at(8)));
    }
  }
  else {
    record_marker_ = false;
    have_hokuyo_data_ = false;

    marker_sub_ = nh_.subscribe("/" + quad_name + "/visualization_marker", 1,
                            &ObstacleCalculator::VisualizationMarkerCallback, this);
    record_sub_ = nh_.subscribe("/" + quad_name + "/obstacle_record", 1,
                            &ObstacleCalculator::RecordMarkerCallback, this);
  }

  odom_sub_ = nh_.subscribe("/" + quad_name + "/uav/cog/odom", 1,
                            &ObstacleCalculator::CalculatorCallback, this);
  obs_pub_ = nh_.advertise<aerial_robot_msgs::ObstacleArray>(
      "/" + quad_name + "/polar_pixel", 1);
  obs_min_dist_pub_ = nh_.advertise<std_msgs::Float64>(
      "/" + quad_name + "/debug/min_obs_distance_with_body", 1);

  theta_list_ = {5,15,25,35,45,55, 65, 75, 85, 95, 105, 115, 125};//should change Theta_Cuts if you change this theta's num
  acc_theta_list_ = {1,4,7,10};
  // phi_list_ = {5};
  wall_x_pos_ = 4.00;
  wall_y_pos_ = 1.75;

  common_theta_ = {90,90,90};
  common_l_ = 0.6;
  common_r_ = 0.2;

  set_collision_point();
}

void ObstacleCalculator::VisualizationMarkerCallback(const visualization_msgs::MarkerArray::ConstPtr &msg){

  if (record_marker_){
    for (const visualization_msgs::Marker &tree_data : msg->markers) {
      if (tree_data.ns=="tree_diameter") continue;
      Eigen::Vector3d tree_pos;
      geometry_msgs::Point position = tree_data.pose.position;
      tree_pos(0) = position.x; //world coodinate
      tree_pos(1) = position.y; //world coodinate
      tree_pos(2) = 0;
      positions_.push_back(tree_pos);

      geometry_msgs::Vector3 scale = tree_data.scale;
      radius_list_.push_back(scale.x/2);
      have_hokuyo_data_ = true;
      record_marker_ = false;
    }
  }
}

void ObstacleCalculator::RecordMarkerCallback(const std_msgs::Empty::ConstPtr &msg){
  record_marker_ = true;
}

void ObstacleCalculator::CalculatorCallback(
    const nav_msgs::Odometry::ConstPtr &msg) {

  Eigen::Vector3d pos;
  tf::pointMsgToEigen(msg->pose.pose.position, pos);
  Eigen::Quaternion<Scalar> quat;
  tf::quaternionMsgToEigen(msg->pose.pose.orientation, quat);
  Eigen::Vector3d euler = quat.toRotationMatrix().eulerAngles(0, 1, 2);
  if (print_yaw_) {
    std::cout << "yaw is " << euler[2] << std::endl;
  }
  // set attitude as the yaw = 0.0
  if (euler[2] > M_PI /2) euler[2] = M_PI;
  else if (euler[2] < -M_PI /2) euler[2] = -M_PI;
  else euler[2] = 0.0;
  Eigen::Matrix3d R = (Eigen::AngleAxis<Scalar>(euler[0], Vector<3>::UnitX()) *
        Eigen::AngleAxis<Scalar>(euler[1], Vector<3>::UnitY()) *
        Eigen::AngleAxis<Scalar>(euler[2], Vector<3>::UnitZ())).toRotationMatrix();
  Eigen::Matrix3d R_T = R.transpose();
  Eigen::Vector3d poll_x(R_T(0, 0), R_T(1, 0), R_T(2, 0));
  Eigen::Vector3d poll_y(R_T(0, 1), R_T(1, 1), R_T(2, 1));
  Eigen::Vector3d poll_z(R_T(0, 2), R_T(1, 2), R_T(2, 2));

  Eigen::Vector3d vel;
  tf::vectorMsgToEigen(msg->twist.twist.linear, vel);
  Eigen::Vector3d omega;
  tf::vectorMsgToEigen(msg->twist.twist.angular, omega);

  std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> converted_positions;
  double min_dist = max_detection_range_;
  size_t call = 0;

  for (const auto &tree_pos : positions_) {
    Eigen::Vector3d converted_pos = R_T * (tree_pos - pos);
    converted_positions.push_back(converted_pos);
    Eigen::Vector2d converted_pos_2d = {(tree_pos - pos)[0], (tree_pos - pos)[1]}; //world coordinate
    min_dist = std::min(min_dist, converted_pos_2d.norm() - radius_list_[call]);
    call++;
  }
  std_msgs::Float64 min_dist_msg;
  min_dist_msg.data = min_dist;
  obs_min_dist_pub_.publish(min_dist_msg);

  Vector<Vision::Theta_Cuts> sphericalboxel =
      getsphericalboxel<Vector<Vision::Theta_Cuts>>(converted_positions, poll_x, poll_y, poll_z, theta_list_, pos);

  Vector<3> vel_2d = {vel[0], vel[1], 0};
  Vector<3> body_vel = R_T * vel_2d;
  Vector<Vision::ACC_Theta_Cuts> acc_sphericalboxel =
    getsphericalboxel<Vector<Vision::ACC_Theta_Cuts>>(converted_positions, poll_x, poll_y, poll_z, acc_theta_list_, pos, body_vel);

  get_common_sphericalboxel(converted_positions, poll_x, poll_y, poll_z, R_T, vel, omega, pos);

  aerial_robot_msgs::ObstacleArray obstacle_msg;
  //   obstacle_msg.header.stamp = ros::Time(state.t);

  obstacle_msg.header = msg->header;
  for (size_t i = 0; i < sphericalboxel.size(); ++i) {
    obstacle_msg.boxel.push_back(sphericalboxel[i]);
  }
  for (size_t i = 0; i < acc_sphericalboxel.size(); ++i) {
    obstacle_msg.acc_boxel.push_back(acc_sphericalboxel[i]);
  }
  for (size_t i = 0; i < Vision::Corner_Num; ++i) {
    obstacle_msg.C_vel_obs.push_back(C_vel_obs_distance_[i]);
  }
  for (size_t i = 0; i < Vision::Rotor_Num; ++i) {
    obstacle_msg.R_vel_obs.push_back(R_vel_obs_distance_[i]);
  }
  if (!from_hokuyo_ || have_hokuyo_data_){
  obs_pub_.publish(obstacle_msg);
  }

  //   if (call == 400) {
  //     std::cout << "v: " << std::endl;
  //     std::cout << v << std::endl;
  //     call = 0;
  //   } else {
  //     call += 1;
  //   }
}

template <typename T>
T ObstacleCalculator::getsphericalboxel(
    const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> &converted_positions,
    const Vector<3> &poll_x, const Vector<3> &poll_y, const Eigen::Vector3d &poll_z, const std::vector<Scalar> &theta_list, Eigen::Vector3d &pos) {

  T obstacle_obs;
  size_t size = theta_list.size();
  for (int t = -(int)size; t < (int)size; ++t) {
    Scalar theta = (t >= 0) ? theta_list[t] : -theta_list[(-t) - 1];  //[deg]
    // for (int p = -Vision::Phi_Cuts / 2; p < Vision::Phi_Cuts / 2; ++p) {
    //   Scalar phi = (p >= 0) ? phi_list_[p] : -phi_list_[(-p) - 1];  //[deg]

      Scalar tcell = theta * (M_PI / 180);
      // Scalar pcell = phi* (M_PI / 180);
      Scalar closest_distance = getClosestDistance(converted_positions, poll_x, poll_y, poll_z, tcell, 0, pos);
      obstacle_obs(size+t) = closest_distance;
    // }
  }
  return obstacle_obs;
}

template <typename T>
T ObstacleCalculator::getsphericalboxel(const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> &converted_positions,
					const Vector<3> &poll_x, const Vector<3> &poll_y, const Eigen::Vector3d &poll_z, const std::vector<Scalar> &theta_list, Eigen::Vector3d &pos, Eigen::Vector3d &body_vel){
  Scalar vel_theta, vel_phi;
  if (body_vel.norm() < vel_calc_boundary_){
    vel_theta = 0;
    vel_phi = 0;
  }
  else {
    vel_theta = std::atan2(body_vel[1], body_vel[0]);
    vel_phi = std::atan(body_vel[2] / std::sqrt(std::pow(body_vel[0], 2) +
                                                       std::pow(body_vel[1], 2)));
  }
  T obstacle_obs;
  size_t size = theta_list.size();
  for (int t = -(int)size; t < (int)size; ++t) {
    Scalar theta = (t >= 0) ? theta_list[t] : -theta_list[(-t) - 1];  //[deg]

      Scalar tcell = theta * (M_PI / 180);
      // Scalar pcell = phi* (M_PI / 180);
      Scalar closest_distance = getClosestDistance(converted_positions, poll_x, poll_y, poll_z, tcell+vel_theta, vel_phi, pos);
      obstacle_obs(size+t) = closest_distance;
    // }
  }
  return obstacle_obs;
}

Scalar ObstacleCalculator::getClosestDistance(
    const std::vector<Vector<3>, Eigen::aligned_allocator<Vector<3>>> &converted_positions,
    const Vector<3> &poll_x, const Vector<3> &poll_y, const Eigen::Vector3d &poll_z, Scalar tcell, Scalar fcell, Eigen::Vector3d &quad_pos) {
  Eigen::Vector3d Cell = getCartesianFromAng(tcell, fcell);
  Eigen::Vector3d quad_wall_pos = quad_pos - Eigen::Vector3d(shift_x_, shift_y_, 0); // obstacle coordinate
  Scalar y_p = calc_dist_from_wall(+1, Cell, wall_y_pos_, poll_y, quad_wall_pos[1]); //[m]

  Scalar y_n = calc_dist_from_wall(-1, Cell, wall_y_pos_, poll_y, quad_wall_pos[1]); //[m]
  Scalar x_p = calc_dist_from_wall(+1, Cell, wall_x_pos_, poll_x, quad_wall_pos[0]); //[m]
  Scalar rmin = std::min(std::min(std::min(y_p, y_n),x_p), max_detection_range_); //[m] from wall
  for (int i = 0; i < positions_.size(); i++) {
    Eigen::Vector3d pos = converted_positions[i];
    Scalar radius = radius_list_[i] + body_r_;
    Eigen::Vector3d alpha = Cell.cross(poll_z);
    Eigen::Vector3d beta = pos.cross(poll_z);
    Scalar a = std::pow(alpha.norm(), 2);
    if (a == 0) continue;
    Scalar b = alpha.dot(beta);
    Scalar c = std::pow(beta.norm(), 2) - std::pow(radius, 2);
    Scalar D = std::pow(b, 2) - a * c;
    if (0 <= D) {
      Scalar dist = (b - std::sqrt(D)) / a;
      if (0 < dist){
      rmin = std::min(dist, rmin);
      }
    }
  }
   return rmin / max_detection_range_;
}

Scalar ObstacleCalculator::calc_dist_from_wall(Scalar sign, const Vector<3>& Cell, const Scalar wall_pos, const Vector<3> &poll, Scalar direstion_pos) const {
  Scalar y_d= (sign*(wall_pos-body_r_) - direstion_pos);
  Scalar cos_theta = Cell.dot(poll);
  if (cos_theta*y_d <= 0) return max_detection_range_;
  else return y_d/cos_theta;
}

Eigen::Vector3d ObstacleCalculator::getCartesianFromAng(Scalar theta,
                                                        Scalar phi) {
  Eigen::Vector3d cartesian(std::cos(theta) * std::cos(phi),
                            std::sin(theta) * std::cos(phi), std::sin(phi));
  return cartesian;
}

std::vector<std::string> ObstacleCalculator::split(std::string &input,
                                                   char delimiter) {
  std::istringstream stream(input);
  std::string field;
  std::vector<std::string> result;
  while (std::getline(stream, field, delimiter)) {
    result.push_back(field);
  }
  return result;
}

void ObstacleCalculator::get_common_sphericalboxel(
  const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> &converted_positions,
  const Vector<3> &poll_x, const Vector<3> &poll_y, const Eigen::Vector3d &poll_z, const Eigen::Matrix3d &R_T,
  const Eigen::Vector3d &vel, const Eigen::Vector3d &omega, Eigen::Vector3d &pos) {
  for (size_t i = 0; i < C_list_.size(); i++) {
    Vector<3> b_p_ref{C_list_[i](0), C_list_[i](1), 0};
    std::vector<Vector<3>, Eigen::aligned_allocator<Vector<3>>> pos_from_corner;
    for (Vector<3> pos : converted_positions) {
      pos_from_corner.push_back(pos - b_p_ref);
    }

    Eigen::Matrix3d R = R_T.transpose();
    Vector<3> w_vel =
      vel + omega.cross(R * b_p_ref);
    Vector<3> w_vel_2d = {w_vel[0], w_vel[1], 0};
    if (w_vel_2d.norm()>vel_calc_boundary_){
    Vector<3> body_vel = R_T * w_vel_2d;
    Scalar vel_theta = std::atan2(body_vel[1], body_vel[0]);
    Scalar vel_phi =
      std::atan(body_vel[2] /
                std::sqrt(std::pow(body_vel[0], 2) + std::pow(body_vel[1], 2)));

    C_vel_obs_distance_[i] =
      getClosestDistance(pos_from_corner, poll_x, poll_y, poll_z, vel_theta, vel_phi, pos) * max_detection_range_;
      // if (C_vel_obs_distance_[i]<-10000){
      //   ROS_INFO("body_vel: %lf,%lf,%lf",body_vel[0],body_vel[1],body_vel[2]);
      //   ROS_INFO("angle: %lf,%lf",vel_theta,vel_phi);
      //   ROS_INFO("C_vel_obs_distance_[i]: %lf",C_vel_obs_distance_[i]);
      }
  else{
    C_vel_obs_distance_[i] = max_detection_range_;
  }
  }
  for (size_t i = 0; i < R_list_.size(); i++) {
    Vector<3> b_p_ref{R_list_[i](0), R_list_[i](1), 0};
    std::vector<Vector<3>, Eigen::aligned_allocator<Vector<3>>> pos_from_corner;
    for (Vector<3> pos : converted_positions) {
      pos_from_corner.push_back(pos - b_p_ref);
    }

    Eigen::Matrix3d R = R_T.transpose();
    Vector<3> w_vel =
      vel + omega.cross(R * b_p_ref);
    Vector<3> w_vel_2d = {w_vel[0], w_vel[1], 0};

    if (w_vel_2d.norm()>vel_calc_boundary_){
    Vector<3> body_vel = R_T * w_vel_2d;
    Scalar vel_theta = std::atan2(body_vel[1], body_vel[0]);
    Scalar vel_phi =
      std::atan(body_vel[2] /
                std::sqrt(std::pow(body_vel[0], 2) + std::pow(body_vel[1], 2)));
    R_vel_obs_distance_[i] =
      getClosestDistance(pos_from_corner, poll_x, poll_y, poll_z, vel_theta, vel_phi, pos) * max_detection_range_;
    }
    else{
    R_vel_obs_distance_[i] = max_detection_range_;
    }
  }
  
}

bool ObstacleCalculator::set_collision_point() {
  Scalar theta_1 = common_theta_[0] * M_PI / 180;
  Scalar theta_2 = common_theta_[1] * M_PI / 180;
  Scalar theta_3 = common_theta_[2] * M_PI / 180;

  Eigen::Vector2d bR2(0.0, 0.0);
  Eigen::Vector2d bC2(-common_l_ / 2, 0.0);
  Eigen::Vector2d bC2TobR1(common_l_ / 2 * (-std::cos(theta_1)),
                           common_l_ / 2 * (std::sin(theta_1)));
  Eigen::Vector2d bR1 = bC2 + bC2TobR1;
  Eigen::Vector2d bC1 = bC2 + 2 * bC2TobR1;

  Eigen::Vector2d bC3(common_l_ / 2, 0.0);
  Eigen::Vector2d bC3TobR3(common_l_ / 2 * (std::cos(theta_2)),
                           common_l_ / 2 * (std::sin(theta_2)));
  Eigen::Vector2d bR3 = bC3 + bC3TobR3;
  Eigen::Vector2d bC4 = bC3 + 2 * bC3TobR3;

  Eigen::Vector2d bC4TobR4(common_l_ / 2 * (std::cos(theta_2 + theta_3)),
                           common_l_ / 2 * (std::sin(theta_2 + theta_3)));
  Eigen::Vector2d bR4 = bC4 + bC4TobR4;
  Eigen::Vector2d bC5 = bC4 + 2 * bC4TobR4;

  Eigen::Vector2d CG = (bR1 + bR2 + bR3 + bR4) / 4;

  Eigen::Vector2d C1 = bC1 - CG;
  C_list_.push_back(C1);
  Eigen::Vector2d R1 = bR1 - CG;
  R_list_.push_back(R1);
  Eigen::Vector2d C2 = bC2 - CG;
  C_list_.push_back(C2);
  Eigen::Vector2d R2 = bR2 - CG;
  R_list_.push_back(R2);
  Eigen::Vector2d C3 = bC3 - CG;
  C_list_.push_back(C3);
  Eigen::Vector2d R3 = bR3 - CG;
  R_list_.push_back(R3);
  Eigen::Vector2d C4 = bC4 - CG;
  C_list_.push_back(C4);
  Eigen::Vector2d R4 = bR4 - CG;
  R_list_.push_back(R4);
  Eigen::Vector2d C5 = bC5 - CG;
  C_list_.push_back(C5);

  return true;
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "common_observation_conversion");
  // The third argument to init() is the name of the node
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  ObstacleCalculator calculator(nh, pnh);

  ros::spin();

  return 0;
}