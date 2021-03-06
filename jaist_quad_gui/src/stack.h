#define BUF_MAX 1024
#define MAX_CPU 128
#include "ardrone_autonomy/Navdata.h"
#include "geometry_msgs/Twist.h"
#include "mainwindow.h"
#include "nav_msgs/Odometry.h"
#include "pcl/io/pcd_io.h"
#include "pcl/point_types.h"
#include "pcl_ros/point_cloud.h"
#include "qcustomplot.h"
#include "ros/ros.h"
#include "ros_launch.h"
#include "ros_thread.h"
#include "sensor_msgs/PointCloud2.h"
#include "std_msgs/Empty.h"
#include "std_msgs/String.h"
#include "std_srvs/Empty.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QLabel>
#include <QMainWindow>
#include <QMetaType>
#include <QMutex>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QThread>
#include <QThreadPool>
#include <QTime>
#include <QVector>
#include <QtAlgorithms>
#include <QtCore>
#include <QtGui/QApplication>
#include <QtGui>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ros/macros.h>
#include <ros/master.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <stdio.h>
#include <unistd.h>
#include <QImage>
#include <QtGlobal>

// path planner hearders 

#include "search.h"
#include "pathPlanner/spline.h"
#include "pathPlanner/Dstar.h"
#include "pathPlanner/path.h"
#include "pathPlanner/pathsmoother.h"
#include <cmath>
#include <QLineF>





