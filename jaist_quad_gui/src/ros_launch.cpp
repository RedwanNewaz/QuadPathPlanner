#include "stack.h"

//https://github.com/karlphillip/GraphicsProgramming/blob/master/cvImage/cvImage.cpp

ros_launch::ros_launch(QObject *parent) :
    QThread(parent)
{

    this->Stop=false;
    navdataCount=0;


}


void ros_launch::run()
{
    std::cout<< "topic in string: "  <<image_topic<<std::endl;
    ros_launch *sensor_subs;
    sensor_subs=new ros_launch(this);

    image_sub=nh_.subscribe(image_topic, 1, &ros_launch::callback_Image,sensor_subs);
    navdata_sub	   = nh_.subscribe(nh_.resolveName("ardrone/navdata"),50, &ros_launch::navdataCb, sensor_subs);
    ukf_sub	   = nh_.subscribe("/odometry/filtered",10, &ros_launch::ukf_localization, sensor_subs);
    orbTraker=nh_.subscribe("ORB_SLAM/Debug",10,&ros_launch::takerCallback, sensor_subs);
    cloud_sub=nh_.subscribe("/ORB_SLAM/PointCloud",10,&ros_launch::pointcloudCallback, sensor_subs);

    connect(sensor_subs,SIGNAL(singal_sensor_sub(QImage)),this,SLOT(slot_ros_launch(QImage)));
    connect(sensor_subs,SIGNAL(nav_battery(double)),this,SLOT(slot_nav_battery(double)));
    connect(sensor_subs,SIGNAL(posRobot(QPoint)),this,SLOT(slot_posRobot(QPoint)));
    connect(sensor_subs,SIGNAL(sig_tracker(QString)),this,SLOT(slot_tracker(QString)));
    connect(sensor_subs,SIGNAL(sub_pointcloud(localmap)),this,SLOT(slot_pointcloud(localmap)));
    qDebug()<<"image subscriber enabled";
    ros::MultiThreadedSpinner();
}

void ros_launch::slot_pointcloud(localmap l){
    emit main_pointcloud(l);
}

void ros_launch::pointcloudCallback(const boost::shared_ptr<const sensor_msgs::PointCloud2>& input)
{
    pcl::PCLPointCloud2 pcl_pc2;
    pcl_conversions::toPCL(*input,pcl_pc2);
    pcl::PointCloud<pcl::PointXYZ>::Ptr temp_cloud(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromPCLPointCloud2(pcl_pc2,*temp_cloud);
    localmap ml;
    for(size_t i=0;i<temp_cloud->points.size();i++)
     {
       QPoint point(temp_cloud->points[i].x,temp_cloud->points[i].y);
       ml.map.push_back(point);
    }
    emit sub_pointcloud(ml);
}

void ros_launch::callback_Image(const sensor_msgs::Image::ConstPtr& msg)
{

    mutex.lock();
        cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::RGB8);
        cv::Mat conversion_mat_ = cv_ptr->image;
        QImage _img(conversion_mat_.data, conversion_mat_.cols, conversion_mat_.rows, conversion_mat_.step[0], QImage::Format_RGB888);
        emit singal_sensor_sub(_img.rgbSwapped());
    mutex.unlock();

    mutex.lock();
        if(Stop)
        {
            qDebug()<<"terminating cv show";
            terminate();
        }
    mutex.unlock();


}

void ros_launch::navdataCb(const ardrone_autonomy::NavdataConstPtr navdataPtr)
{
    if(navdataCount%10==0)
    {
        double battery_level=navdataPtr->batteryPercent;
        emit nav_battery(battery_level);
    }

    navdataCount++;
}

//robot position
void ros_launch::ukf_localization(const nav_msgs::OdometryConstPtr Odom_msg)
{
    double x=Odom_msg->pose.pose.position.x, y=Odom_msg->pose.pose.position.y;
    QPoint robot(x,y);
    emit posRobot(robot);
}

void ros_launch::slot_nav_battery(double msg)
{
    emit ardrone_battery(msg);
}

void ros_launch::slot_ros_launch(const QImage &_img)
{

    emit ImageQ(_img);
}

void ros_launch::takerCallback(const std_msgs::String &msg){
    std::stringstream ss;
    ss<<msg;
    QString str= QString::fromStdString(ss.str());
    emit sig_tracker(str);
}

void ros_launch::slot_tracker(QString msg){
    emit main_sig_tracker(msg);
}

//communicate to main script

void ros_launch::slot_posRobot(QPoint msg){
   emit position(msg);

}
