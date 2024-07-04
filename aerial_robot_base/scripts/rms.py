#!/usr/bin/env python
import rospy

from std_msgs.msg import Empty
from std_msgs.msg import Int8
from std_msgs.msg import UInt16
from aerial_robot_msgs.msg import PoseControlPid
from visualization_msgs.msg import MarkerArray

import sys, select, termios, tty, math

msg = """

s: start to subscribe the topic regarding to control errors, and start to calculate the RMS
h: stop the calculate the output the RMS results.
"""

def cb(data):
        global start_flag

        true_tree_pos_x = -0.2
        true_tree_pos_y = -0.15
        true_tree_radius = 0.25

        marker_id = 2
        if start_flag == True:
                global pose_cnt
                global tree_squared_errors_sum
                pose_cnt = pose_cnt + 1

                error_tree_pos_x = data[marker_id].pose.position.x - true_tree_pos_x
                error_tree_pos_y = data[marker_id].pose.position.y - true_tree_pos_y
                error_tree_radius = float(data[marker_id].text) - true_tree_radius

                tree_squared_errors_sum[0] = tree_squared_errors_sum[0] + error_tree_pos_x * error_tree_pos_x
                tree_squared_errors_sum[1] = tree_squared_errors_sum[1] + error_tree_pos_y * error_tree_pos_y
                tree_squared_errors_sum[2] = tree_squared_errors_sum[2] + error_tree_radius * error_tree_radius

                rms = [math.sqrt(i / pose_cnt) for i in tree_squared_errors_sum]
                #rospy.loginfo("RMS errors of pos: [%f, %f, %f]; rot: [%f, %f, %f]", rms[0], rms[1], rms[2], rms[3], rms[4], rms[5])

def getKey():
        tty.setraw(sys.stdin.fileno())
        select.select([sys.stdin], [], [], 0)
        key = sys.stdin.read(1)
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
        return key

if __name__=="__main__":
        settings = termios.tcgetattr(sys.stdin)

        start_flag = False
        msg_cnt = 0

        tree_squared_errors_sum = [0] * 3
        pose_cnt = 0

        rospy.Subscriber("visualization_marker", MarkerArray, cb)

        rospy.init_node('rms_error')

        print(msg)
        try:
                while(True):
                        key = getKey()

                        if key == 's':
                                rospy.loginfo("start to calculate RMS errors")
                                start_flag = True
                        if key == 'h':
                                rospy.loginfo("stop calculation")

                                rms = [0] * 3
                                if pose_cnt > 0:
                                        rms = [math.sqrt(i / pose_cnt) for i in tree_squared_errors_sum]

                                rospy.loginfo("RMS of tree_pos errors: [%f, %f], radius_errors: [%f]", rms[0], rms[1], rms[2])

                                start_flag = False

                                pose_cnt = 0
                                tree_squared_errors_sum = [0] * 3

                        else:
                                if (key == '\x03'):
                                        break
                        rospy.sleep(0.001)

        except Exception as e:
                print(e)
                print(repr(e))

        finally:
                termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)


