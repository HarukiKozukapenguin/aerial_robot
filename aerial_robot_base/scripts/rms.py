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

class TreeDb:
        shift_x = -2.0
        shift_y = -0.25
        def __init__(pos_x, pos_y, radius, marker_id):
                self.pos_x:float = pos_x + shift_x
                self.pos_y:float = pos_y + shift_y
                self.radius:float = radius
                self.marker_id:int = marker_id
        
def cb(data):
        global start_flag

        if start_flag == True:
                global pose_cnt
                global tree_squared_errors_sum_list
                pose_cnt = pose_cnt + 1

                for i, tree_db in emurate(tree_db_list):
                        true_tree_pos_x = tree_db.pos_x
                        true_tree_pos_y = tree_db.pos_y
                        true_tree_radius = tree_db.radius
                        marker_id = tree_db.marker_id
                        
                        error_tree_pos_x = data.markers[marker_id].pose.position.x - true_tree_pos_x
                        error_tree_pos_y = data.markers[marker_id].pose.position.y - true_tree_pos_y
                        error_tree_radius = float(data.markers[marker_id].text) - true_tree_radius

                        tree_squared_errors_sum_list[i][0] = tree_squared_errors_sum_list[i][0] + error_tree_pos_x * error_tree_pos_x
                        tree_squared_errors_sum_list[i][1] = tree_squared_errors_sum_list[i][1] + error_tree_pos_y * error_tree_pos_y
                        tree_squared_errors_sum_list[i][2] = tree_squared_errors_sum_list[i][2] + error_tree_radius * error_tree_radius

                        # rms = [math.sqrt(i / pose_cnt) for i in tree_squared_errors_sum]
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

        pose_cnt = 0

        global tree_db_list
        global tree_squared_errors_sum_list
        tree_db_list = []
        tree_db_list.append(TreeDb(1.2,0.5,0.25,0))
        tree_db_list.append(TreeDb(2.5,-0.9,0.25,0))
        tree_squared_errors_sum_list = [[0] * 3] * len(tree_db_list)


        rospy.Subscriber("/multirotor/visualization_marker", MarkerArray, cb)

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
                                for j, tree_squared_erros_sum in emurate(tree_squared_errors_sum_list):
                                        rms = [0] * 3
                                        if pose_cnt > 0:
                                                rms = [math.sqrt(i / pose_cnt) for i in tree_squared_errors_sum]

                                        rospy.loginfo("(RMS of marker_id is %d) tree_pos errors: [%f, %f], radius_errors: [%f]", tree_db_list[j].marker_id, rms[0], rms[1], rms[2])

                                start_flag = False

                                pose_cnt = 0
                                tree_squared_errors_sum_list = [[0] * 3] * len(tree_squared_errors_sum_list)

                        else:
                                if (key == '\x03'):
                                        break
                        rospy.sleep(0.001)

        except Exception as e:
                print(e)
                print(repr(e))

        finally:
                termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)


