#ifndef ROS_LAUNCH_H
#define ROS_LAUNCH_H


#include "ros/ros.h"
#include "ardrone_autonomy/Navdata.h"
#include "nav_msgs/Odometry.h"
#include "std_msgs/String.h"

//point cloud header file
#include "pcl/io/pcd_io.h"
#include "pcl/point_types.h"
#include "pcl_ros/point_cloud.h"
#include "sensor_msgs/PointCloud2.h"
#include <ros/macros.h>
#include <sensor_msgs/Image.h>
#include <QThread>
#include <QImage>
#include <QDebug>
#include <QMutex>
#include <image_transport/image_transport.h>
#include <opencv2/core/core.hpp>



//Q_DECLARE_METATYPE
typedef struct{
    QVector<QPoint> map;
} localmap;
Q_DECLARE_METATYPE(localmap)
class ros_launch : public QThread
{
    Q_OBJECT

public:
    explicit ros_launch(QObject *parent = 0);
    void run();
    void callback_Image(const sensor_msgs::Image::ConstPtr& );
    void navdataCb(const ardrone_autonomy::NavdataConstPtr navdataPtr);
    void ukf_localization(const nav_msgs::OdometryConstPtr Odom_msg);
    void pointcloudCallback(const boost::shared_ptr<const sensor_msgs::PointCloud2>& input);
    void takerCallback(const std_msgs::String &msg);
    bool Stop;
    std::string image_topic;


signals:
     void ImageQ(const QImage&);
     void singal_sensor_sub(const QImage&);
     void ardrone_battery(double);
     void nav_battery(double);
     void posRobot(QPoint);
     void position(QPoint);
     void sub_pointcloud(localmap);
     void main_pointcloud(localmap);
     void sig_tracker(QString);
     void main_sig_tracker(QString);

    
public slots:
     void slot_ros_launch(const QImage&);
     void slot_nav_battery(double);
     void slot_posRobot(QPoint);
     void slot_tracker(QString);
     void slot_pointcloud(localmap);

private:
     ros::NodeHandle nh_;
     ros::Subscriber image_sub;
     ros::Subscriber ukf_sub;
     ros::Subscriber navdata_sub;
     ros::Subscriber orbTraker;
     ros::Subscriber cloud_sub;
     unsigned int navdataCount;
     QMutex mutex;
     QImage _image;
     QVector<double>robot_x,robot_y,robot_z;
     localmap m_l;


};

#endif // ROS_LAUNCH_H
