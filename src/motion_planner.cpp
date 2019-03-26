#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/Point.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "multi_agent_planner/get_plan.h"
#include "multi_agent_planner/agent_info.h"

using std::vector;
using std::cout;
using std::endl;

class Motion_Planner
{
public:
    Motion_Planner(ros::NodeHandle *node_handle);
private:
    ros::NodeHandle node;
    ros::Publisher pub_marker;
    ros::Subscriber sub_agent_pose;
    ros::ServiceServer srv_get_plan;
    void create_roadmap();
    bool get_plan(multi_agent_planner::get_plan::Request &req, multi_agent_planner::get_plan::Response &res);
    void agent_start_pose_callback(const multi_agent_planner::agent_info &msg);
    vector<vector<geometry_msgs::Point>> archived_paths;
    vector<multi_agent_planner::agent_info> agent_start_poses;
};

Motion_Planner::Motion_Planner(ros::NodeHandle *node_handle)
    : node(*node_handle)
{
    pub_marker = node.advertise<visualization_msgs::Marker>("visualization_marker", 1);
    sub_agent_pose = node.subscribe("/agent_feedback", 100, &Motion_Planner::agent_start_pose_callback, this);
    srv_get_plan = node.advertiseService("/get_plan", &Motion_Planner::get_plan, this);

    create_roadmap();
}

bool Motion_Planner::get_plan(multi_agent_planner::get_plan::Request &req, multi_agent_planner::get_plan::Response &res)
{
    geometry_msgs::Pose2D start_pose, goal_pose;
    vector<geometry_msgs::Point> path_list;
    goal_pose = req.goal_pose;
    bool found = false;
    for (auto agent : agent_start_poses)
    {
        if (agent.serial_id == req.serial_id)
        {
            start_pose = agent.start_pose;
            found = true;
            break;
        }
    }
    if (!found)
    {
        ROS_ERROR("That agent does not yet exist.");
    }
    return true;

}

void Motion_Planner::agent_start_pose_callback(const multi_agent_planner::agent_info &msg)
{
    bool found = false;
    for (size_t i{0}; i < agent_start_poses.size(); i++)
    {
        if (agent_start_poses.at(i).serial_id == msg.serial_id)
        {
            agent_start_poses.at(i) = msg;
            found = true;
            break;
        }
    }
    if (!found)
    {
        agent_start_poses.push_back(msg);
    }
}

void Motion_Planner::create_roadmap()
{
    visualization_msgs::Marker marker;
    marker.header.frame_id = "/world";
    marker.header.stamp = ros::Time();
    marker.ns = "sphere_list";
    marker.id = 0;
    marker.type = visualization_msgs::Marker::SPHERE_LIST;
    marker.action = visualization_msgs::Marker::ADD;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = 0.25;
    marker.scale.y = 0.25;
    marker.scale.z = 0.25;
    marker.color.r = 1.0;
    marker.color.a = 1.0;
    marker.lifetime = ros::Duration();

    for (size_t i {0}; i <= 10; i++)
        for (size_t j{0}; j <= 10; j++)
        {
            geometry_msgs::Point p;
            p.x = i;
            p.y = j;
            marker.points.push_back(p);
        }
    while (pub_marker.getNumSubscribers() < 1)
     {
       if (!ros::ok())
       {
         break;
       }
       ROS_WARN_ONCE("Please create a subscriber to the marker");
       sleep(1);
     }
    pub_marker.publish(marker);
}

int main( int argc, char** argv )
{
    ros::init(argc, argv, "nodes");
    ros::NodeHandle n;
    Motion_Planner mp(&n);
    ros::spin();
    return 0;
}
