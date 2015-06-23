#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "stack.h"


typedef struct {
    QVector<double>x,y;
}usermap;



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void paintEvent(QPaintEvent* e);

//    pathplanner function

        void pointDraw();
        void mapDraw();
        void searchSpace();
        void populateMap();
        void robotView(QPointF);

        path *plan;


        QCPCurve *fermatSpiral1, *trajectoryLine;
        uint32_t scriptCount;
        float interval;
        struct user{
            double x;
            double y;
            double z;
        }cmd,robot;
        struct property{
            double xmin,xmax;
            double ymin,ymax;
        }lim;
        double resolution;
        int line;
        QVector<QPoint> map,search_space;
        QPoint R;


    ~MainWindow();
    ros_thread *drone_thread;
    ros_launch *sensor_subs;
    QTimer *timer;

    
private slots:
    void initialization();

    void on_buttonTakeOff_clicked();

    void on_buttonLand_clicked();

    void on_buttonMoveit_clicked();

    void on_buttonGraph_clicked();

    void on_buttonToggle_clicked();

    void on_checkVO_stateChanged(int arg1);

    void updateBattery(double status);

    void on_buttonStart_clicked();

    void on_buttonStop_clicked();

    void notificationDisplay(QString);

    void on_aDemo_stateChanged(int arg1);

    void on_bDriver_stateChanged(int arg1);

    void on_cSLAM_stateChanged(int arg1);

    void on_dStateDisplay_stateChanged(int arg1);

    void on_ePathPlanner_stateChanged(int arg1);

    void on_topics_currentIndexChanged(const QString &arg1);

    int read_fields (FILE *fp, unsigned long long int *fields);



    void on_buttonRecord_clicked();

    void on_buttonLSD_SLAM_clicked();

    void trackerCallback(QString);

    void setupPlot();


    void on_findPath_clicked();

    void slot_cloudread(localmap);

private:
    Ui::MainWindow *ui;
    bool toggle_checked;
    int  battery;
    int displayCount;
    bool on_fly_status;
    QString notification,raw_notification;
    QThreadPool *pool;
    QList<QString> processlist;
    QList<QString> action;
    QList<QString> pathFile;
    QList<QString> pid_list;
    QStringList topicList;
    QStringList executionList;
    QProcess *process;
    QProcess *process_action;
    QProcess *rosbag_recorder;
    QProcess *lsd_slam_viewer;
    QProcess *lsd_slam_core;
    QImage *_image;
    QMutex mutex;
    bool cpu_usage_active;
    bool rosbag_recorder_active;
    bool lsd_slam_enable;

    QTime t;

    QVector<double>robot_x, robot_y, map_y;
    FILE *fp;
    unsigned long long int fields[10], total_tick[MAX_CPU], total_tick_old[MAX_CPU], idle[MAX_CPU], idle_old[MAX_CPU], del_total_tick[MAX_CPU], del_idle[MAX_CPU];
    int update_cycle, i, cpus, count;
    double percent_usage;



public slots:
    void sub_write(const QImage&);
    void onNumberChanged(int);
    void timeChanged();
    void EnableSubscriber();
    void graphUpdate(QPoint);

protected:
    bool moveit_enable;



};

#endif // MAINWINDOW_H
