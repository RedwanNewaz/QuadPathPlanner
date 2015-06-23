#include "stack.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    MainWindow::initialization();

    resolution=20; //map resolution
    QPoint map_max,map_min;
        map_max.setX(-100);
        map_max.setY(100);
        map_min.setX(100);
        map_min.setY(100);
    map.push_back(map_max);
    map.push_back(map_min);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialization()
{
    //    ros publisher thread (mainly deploy for controlling real ardrone by publishing ros message)

        displayCount=0;
        ui->setupUi(this);
        battery=0;
        on_fly_status=false;
        moveit_enable=false;
        drone_thread=new ros_thread(this);
        /*--------------------------------------------------------------------------------------------*/


    //    cpu clock update and cpu usage are shown by this function

        timer=new QTimer(this);
        update_cycle = 0;
        cpus = 0;
        fp = fopen ("/proc/stat", "r");
        if (fp == NULL)
         {
           qDebug()<<"Error!!! cpu file can't read";
            cpu_usage_active=false;
         }
        else{
             timer->start(1000);
          cpu_usage_active=true;
         }
        while (MainWindow::read_fields (fp, fields) != -1)
         {
           for (i=0, total_tick[cpus] = 0; i<10; i++)
           { total_tick[cpus] += fields[i]; }
           idle[cpus] = fields[3]; /* idle ticks index */
           cpus++;
         }
        connect(timer,SIGNAL(timeout()),this,SLOT(timeChanged()));
        /*--------------------------------------------------------------------------------------------*/

    //    ros subscriber thread (mainly related to ros_launch thread)

        sensor_subs=new ros_launch(this);
        _image=new QImage;
        connect(sensor_subs,SIGNAL(ImageQ(QImage)),this,SLOT(sub_write(QImage)));
        /*--------------------------------------------------------------------------------------------*/

    //    directories of launch files and convert those to processes

        QFile file("/home/redwan/Desktop/moveIT/moveit/src/Ardrone/path.cfg");
        if(!file.open(QIODevice::ReadOnly)) {
            qDebug()<<"Error No file !";
        }
        QTextStream in(&file);
        int lineCount=0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            if(lineCount<2)action.append(line);
            else pathFile.append(line);
            lineCount++;
        }
        file.close();
        qDebug()<<"path size: "<<pathFile.size();
        /*--------------------------------------------------------------------------------------------*/

    //    manipulate some basic inputs to the system

        connect(sensor_subs,SIGNAL(ardrone_battery(double)),this,SLOT(updateBattery(double)));
        topicList<<"/ORB_SLAM/Frame"<<"/camera/image_raw"<<"/ardrone/image_raw";
        ui->topics->addItems(topicList);
        ui->checkImage->setChecked(true);
        ui->simulation->setChecked(true);
        ui->aDemo->setChecked(true);
        ui->bDriver->setChecked(true);
        ui->cSLAM->setChecked(true);
        ui->dStateDisplay->setChecked(true);
        ui->displayBattery->setPalette(Qt::red);
        /*--------------------------------------------------------------------------------------------*/

        // tacker
        connect(sensor_subs,SIGNAL(main_sig_tracker(QString)),this,SLOT(trackerCallback(QString)));
        //pointcloud
        connect(sensor_subs,SIGNAL(main_pointcloud(localmap)),this,SLOT(slot_cloudread(localmap)));


     /*--------------------------------------------------------------------------------------------*/
    //    utilities
        rosbag_recorder =new QProcess(this);
        rosbag_recorder_active=true;
        lsd_slam_enable=false;

        /*--------------------------------------------------------------------------------------------*/
        ui->plot->axisRect()->setMinimumSize(300, 180);
//         connect(sensor_subs,SIGNAL(position(QVector,QVector)),this,SLOT(graphUpdate(QVector,QVector)));
         connect(sensor_subs,SIGNAL(position(QPoint)),this,SLOT(graphUpdate(QPoint)));

        plan=new path;
        resolution=0.2; //map resolution
        interval=1;
        setupPlot();
        populateMap();
        mapDraw();


}

void MainWindow::notificationDisplay(QString msg)
{
    displayCount++;
    if(displayCount<2)
        notification+=QString::number(displayCount)+": "+ msg;
    else
        notification+="\n"+QString::number(displayCount)+": "+msg;

    ui->textStatus->setPlainText(notification);
}

void MainWindow::on_buttonTakeOff_clicked()
{

    if (!on_fly_status){       
        MainWindow::notificationDisplay("Jaist Quadrotor is taking Off");
        on_fly_status=true;
        drone_thread->sendTakeoff();
    }
    else
        MainWindow::notificationDisplay("Quad is already in flying mode");

}

void MainWindow::on_buttonLand_clicked()
{

    if(on_fly_status){
        raw_notification="Landing...";
        on_fly_status=false;
         drone_thread->sendLand();
    }
    else
         raw_notification="Quad is already in Landing mode";
      MainWindow::notificationDisplay(raw_notification);
}

//moveit_button
void MainWindow::on_buttonMoveit_clicked()
{

    if(!moveit_enable)
    {
        moveit_enable=true;
        if(!processlist.empty())
        {


            for(int i=0;i<processlist.size();i++)
            {   if(i==0)
                raw_notification+="/*configured for:_"+processlist[i]+"\n";
                else raw_notification+="\t\t/*configured for:_"+processlist[i]+"\n";
            }

            MainWindow::notificationDisplay(raw_notification);
            for(int i=0;i<processlist.size();i++)
            {
                QString str=processlist[i];
                 QStringList list2 = str.split("_", QString::SkipEmptyParts);
                 int num=list2.first().toInt();
                 if(num<pathFile.size())
                 {
                     executionList<<pathFile[num];
                 }
                 else qDebug()<<"ERROR! No path found!";
            }

        }
        else
            qDebug()<<" process list is empty";

        process =new QProcess;
        qDebug()<<executionList;
        process->start("roslaunch",executionList);
        raw_notification="Moveit is configured for operation...";
    }
    else
         raw_notification="Did you shutdown the process?";


    MainWindow::notificationDisplay(raw_notification);
    qDebug()<<"processes are "<<process->state();

}

void MainWindow::on_buttonGraph_clicked()
{
    raw_notification="RQT graph will show the graph...";
     MainWindow::notificationDisplay(raw_notification);
}

void MainWindow::on_buttonToggle_clicked()
{

    if(toggle_checked){
        raw_notification="toggle to bottom camera...";
        toggle_checked=false;
    }
    else{
        raw_notification="toggle to front camera...";
        toggle_checked=true;
    }
    drone_thread->sendToggleCam();
    MainWindow::notificationDisplay(raw_notification);
}

void MainWindow::on_checkVO_stateChanged(int arg1)
{

    if(arg1==2)
        raw_notification="visual odometry is contributing for hovering";
    else
        raw_notification="On-board hovering is running action";
    MainWindow::notificationDisplay(raw_notification);
}

void MainWindow::updateBattery(double status)
{

    ui->displayBattery->display(status);


}

//start_button
void MainWindow::on_buttonStart_clicked()
{
    //    Started
    raw_notification="Ros thread is started";

    if (ui->experiment->isChecked())
    {
        action.removeLast();
    }
    if (ui->simulation->isChecked())
    {
        action.removeFirst();
    }

    if (moveit_enable)
    {
        if (!ui->test->isChecked() && action.size()<2)
        {
            qDebug()<<action;
            process_action =new QProcess(this);
            process_action->start("roslaunch",action);
        }
        if (ui->test->isChecked())
        {
            qDebug()<<"rqt_graph";
            process_action =new QProcess(this);
            process_action->start("rqt_graph");
            raw_notification="/* configured for test\n";
        }
    }
    MainWindow::notificationDisplay(raw_notification);

    MainWindow::EnableSubscriber();

    sleep(1);
    drone_thread->start();
    t.start();
}

//stop_button
void MainWindow::on_buttonStop_clicked()
{
     qDebug()<<"Application closing request accepted";
     MainWindow::notificationDisplay("Application closing request accepted");
    if (timer->isActive()){
        timer->stop();
        sleep(1);
         MainWindow::notificationDisplay("timer is stopped");
    }

    if(cpu_usage_active){
        fclose (fp);
        sleep(1);
        MainWindow::notificationDisplay("cpu Usage is closed");
        qDebug()<<"cpu Usage closed";
        cpu_usage_active-false;
    }

    if(!moveit_enable)
    {
        raw_notification="ERROR!!!... moveit is not configured";
    }
    else
    {
        process->terminate();
        process_action->terminate();
        sensor_subs->Stop=true;
        sensor_subs->terminate();
        qDebug()<<"terminating action "<<process_action->state()<<"k";
        if (process_action->state()==0 && process->state()==0)
           {
            on_fly_status=false;
            moveit_enable=false;
            raw_notification="all processes are terminated";
        }
        else
            raw_notification="press close button again";

    }

    if(lsd_slam_enable)
    {
        lsd_slam_core->terminate();
        if(ui->lsd_graph->isChecked())
        lsd_slam_viewer->terminate();

        lsd_slam_enable=false;
    }

    if(ros::ok())
    {
        qDebug()<<"terminating ROS";
        ros::shutdown();
        sleep(1);
    }
    QProcess::startDetached(QApplication::applicationFilePath());
    exit(1);

}

void MainWindow::onNumberChanged(int Number)
{

    MainWindow::updateBattery(Number);
}

void MainWindow::timeChanged()
{
    QDateTime display_time;
    ui->runTime->setDateTime(display_time.currentDateTime());

     fseek (fp, 0, SEEK_SET);
         fflush (fp);
//         printf ("[Update cycle %d]\n", update_cycle);
         for (count = 0; count < cpus; count++)
         {
           total_tick_old[count] = total_tick[count];
           idle_old[count] = idle[count];

           if (!MainWindow::read_fields (fp, fields))
           { return ; }

           for (i=0, total_tick[count] = 0; i<10; i++)
           { total_tick[count] += fields[i]; }
           idle[count] = fields[3];

           del_total_tick[count] = total_tick[count] - total_tick_old[count];
           del_idle[count] = idle[count] - idle_old[count];

           percent_usage = ((del_total_tick[count] - del_idle[count]) / (double) del_total_tick[count]) * 100;
           if (count == 0)
           { //printf ("Total CPU Usage: %3.2lf%%\n", percent_usage);
            ui->cpu_usage->setValue(percent_usage);
           }
//           else
//           { printf ("\tCPU%d Usage: %3.2lf%%\n", count - 1, percent_usage); }
         }
         update_cycle++;
//         printf ("\n");




}

void MainWindow::on_aDemo_stateChanged(int arg1)
{
    if(arg1==2)
        processlist.append("0_demo");
    else
    {
        if(processlist.contains("0_demo"))
            processlist.removeOne("0_demo");
    }
}

void MainWindow::on_bDriver_stateChanged(int arg1)
{
    if(arg1==2)
        processlist.append("1_driver");
    else
    {
        if(processlist.contains("1_driver"))
            processlist.removeOne("1_driver");
    }
}

void MainWindow::on_cSLAM_stateChanged(int arg1)
{
    if(arg1==2)
        processlist.append("2_slam");
    else
    {
        if(processlist.contains("2_slam"))
            processlist.removeOne("2_slam");
    }
}

void MainWindow::on_dStateDisplay_stateChanged(int arg1)
{
    if(arg1==2)
        processlist.append("3_state");
    else
    {
        if(processlist.contains("3_state"))
            processlist.removeOne("3_state");
    }
}

void MainWindow::on_ePathPlanner_stateChanged(int arg1)
{
    if(arg1==2)
        processlist.append("4_path");
    else
    {
        if(processlist.contains("4_path"))
            processlist.removeOne("4_path");
    }
}

void MainWindow::EnableSubscriber()
{
    raw_notification+="\n\t\t/*image subscription is enabled";
    QString en= topicList[ui->topics->currentIndex()];
    std::string img_sub_topic=en.toUtf8().constData();
    if(ui->checkImage->checkState())
    {
        sensor_subs->image_topic=img_sub_topic;
        sleep(1);
        MainWindow::notificationDisplay(raw_notification);
        sensor_subs->start();
    }
}

void MainWindow::on_topics_currentIndexChanged(const QString &arg1)
{
    qDebug()<<"topic changed to: "<<arg1;

}

void MainWindow::sub_write(const QImage &frame)
{
    ui->image_view->setPixmap(QPixmap::fromImage(frame).scaled(frame.width(), frame.height(),Qt::KeepAspectRatio));

}

void MainWindow::paintEvent(QPaintEvent* e)
{

}

int MainWindow::read_fields (FILE *fp, unsigned long long int *fields)
{
  int retval;
  char buffer[BUF_MAX];


  if (!fgets (buffer, BUF_MAX, fp))
  { perror ("Error"); }
  /* line starts with c and a string. This is to handle cpu, cpu[0-9]+ */
  retval = sscanf (buffer, "c%*s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
                            &fields[0],
                            &fields[1],
                            &fields[2],
                            &fields[3],
                            &fields[4],
                            &fields[5],
                            &fields[6],
                            &fields[7],
                            &fields[8],
                            &fields[9]);
  if (retval == 0)
  { return -1; }
  if (retval < 4) /* Atleast 4 fields is to be read */
  {
    fprintf (stderr, "Error reading /proc/stat cpu field\n");
    return 0;
  }
  return 1;
}


void MainWindow::on_buttonRecord_clicked()
{
    if(rosbag_recorder->state()==0 )
    {
        rosbag_recorder->setWorkingDirectory("/home/redwan/Desktop/moveIT/moveit/src/Ardrone/jaistQuad_launch/recordFlight");
        qDebug()<<rosbag_recorder->workingDirectory();
//        rosbag_recorder->start("rosbag",QStringList()<<"record"<<"-a");
        rosbag_recorder->start("rqt_bag");
        raw_notification="recorder started at dir: ";

    }
    else
    {
        rosbag_recorder->terminate();
        raw_notification="recorder stopped "+ QString::number(rosbag_recorder->exitStatus());
    }
    MainWindow::notificationDisplay(raw_notification);
}

void MainWindow::on_buttonLSD_SLAM_clicked()
{
    if(!lsd_slam_enable)
    {
        raw_notification="lsd_slam is configured\n\t\t";
        if(ui->lsd_graph->isChecked())
        {
            lsd_slam_viewer=new QProcess(this);
            QStringList viewer_cmd;
            viewer_cmd<<"lsd_slam_viewer"<<"viewer";
            lsd_slam_viewer->start("rosrun",viewer_cmd);
            raw_notification+="/*Graph view is enabled";
        }
        lsd_slam_core=new QProcess(this);
        QStringList core_cmd;
        core_cmd<<"lsd_slam_core"<<"live_slam"<<"/image:=/camera/image_raw"<<"_calib:=/home/redwan/Desktop/Navigation/lsd.cfg";
        lsd_slam_core->start("rosrun",core_cmd);
        lsd_slam_enable=true;
        MainWindow::notificationDisplay(raw_notification);
    }

}

void MainWindow::setupPlot()
{
    line=0;
   // add two new graphs and set their look:
   ui->plot->addGraph();
   ui->plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
   ui->plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue

   ui->plot->addGraph();
   ui->plot->graph(1)->setPen(QPen(Qt::green)); // line color red for second graph

    fermatSpiral1 = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
    ui->plot->addPlottable(fermatSpiral1);
    fermatSpiral1->setPen(QPen(Qt::red));
    fermatSpiral1->setBrush(QBrush(QColor(255, 0, 0, 20)));

    //viuslaize robot triangle instead of trajectory
    trajectoryLine = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
    ui->plot->addPlottable(trajectoryLine);
    trajectoryLine->setPen(QPen(Qt::black));
    trajectoryLine->setBrush(QBrush(QColor(0, 0, 0, 20)));

    // generate some points of data (y0 for first, y1 for second graph):
   QVector<double> x(250), y0(250), y1(250);
   for (int i=0; i<250; ++i)
   {
     x[i] = i;
     y0[i] = qExp(-i/150.0)*qCos(i/10.0); // exponentially decaying cosine
     y1[i] = qExp(-i/150.0);              // exponential envelope
   }
   ui->plot->xAxis2->setVisible(true);
   ui->plot->xAxis2->setTickLabels(false);
   ui->plot->yAxis2->setVisible(true);
   ui->plot->yAxis2->setTickLabels(false);
   // make left and bottom axes always transfer their ranges to right and top axes:
   connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
   connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));
   // pass data points to graphs:
   ui->plot->graph(0)->setData(x, y0);
   ui->plot->graph(1)->setData(x, y1);
   ui->plot->graph(0)->rescaleAxes();
   ui->plot->graph(1)->rescaleAxes(true);
   ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

}

void MainWindow::graphUpdate(QPoint r)
{
    QString ss;
    ss="x: "+QString::number(r.x())+"\ty: "+QString::number(r.y());
    ui->positionLabel->setText(ss);
    robot.x=r.x();
    robot.y=r.y();
//    ui->plot->graph(1)->setData(robot_x, robot_y);
//    ui->plot->graph(1)->rescaleAxes();
//    ui->plot->replot();

}

void MainWindow::trackerCallback(QString msg){

//    QFile file("result_tau_th.txt");
//    file.open(QIODevice::Append | QIODevice::Text);
//    QStringList list=msg.split(QRegExp("\n"));

//    if(file.isOpen()){
//          QTextStream outStream(&file);
//          outStream<<list[2]<<"\t"<<t.toString()<<"\n";
//          qDebug()<<list<<"\t"<<t.elapsed();
//    }
//    file.close();




}


//path planning-------------------------------------------------------------------------------------------------------------------------------------------


usermap map2vector(QVector<QPoint>localmap){
   usermap map;
   foreach(QPoint p,localmap){
       map.x.push_back(p.x());
       map.y.push_back(p.y());
   }
   return map;
}

////function description-----------
void MainWindow::robotView(QPointF p){
    double r=1;
    double rot=(M_PI/360)*(60+180);
    QVector<double>x,y;
    for(float theta=2*M_PI+rot;theta>=rot;theta-=2*M_PI/3)
    {
        x.push_back(p.x()+r*cos(theta));
        y.push_back(p.y()+r*sin(theta));
    }
    x.push_front(p.x()+r*cos(2*M_PI+rot));
    y.push_front(p.y()+r*sin(2*M_PI+rot));

    trajectoryLine->setData(x, y);
    ui->plot->replot();
}

void MainWindow::populateMap(){
   QPoint map_max,map_min;
       map_max.setX(-100);
       map_max.setY(10);
       map_min.setX(100);
       map_min.setY(10);

       map.clear();

//        make a sine wave to indicate the boundary
       //            calculate R
       float R=plan->pointDist(QPoint(0,0),QPoint(-100,10));
       float phi=0;
       for (float theta=0;theta<=2*M_PI;theta+=resolution){
           double x=R*cos(theta);
           double y=R*sin(theta);
           if(x>=-100 && x<=100 && y>10){
               phi+=resolution;
               phi=fmod(phi,2*M_PI);
               y+=sin(phi);

               map.push_back(QPoint(x,y));

           }
       }
}

void MainWindow::searchSpace(){
   QString msg;
   msg="search\t";
   QPoint G(cmd.x,cmd.y);
   robotView(G);

   bool result=plan->input(R,G);
   if(!result){
       msg+="result not found!!!";
       notificationDisplay(msg);
       return;
   }
   search_space=plan->output();
   usermap planpath=map2vector(search_space);
   fermatSpiral1->setData(planpath.x, planpath.y);
   ui->plot->replot();


//    path smoothing
   pathsmoother *smooth;
   smooth =new pathsmoother;
   int dirChange=smooth->DirChange(search_space);


   QVector<double>pathx,pathy;
   mutex.lock();
   foreach (QPointF p, smooth->splinePath()){
       pathx.push_back(p.x());
       pathy.push_back(p.y());
       robotView(p);
//        QThread::msleep(interval/3);
   }
   mutex.unlock();

   trajectoryLine->setData(pathx, pathy);
   ui->plot->replot();


   msg+="result "+QString::number(dirChange);

   notificationDisplay(msg);



}

void MainWindow:: pointDraw()
{
//    taking user input
    QString x1=ui->xBox->toPlainText();
    QString y1=ui->yBox->toPlainText();
    double center_x=x1.toDouble();
    double center_y=y1.toDouble();
    cmd.x=center_x;
    cmd.y=center_y;

//     draw a line
    QVector<double>x,y;
    x.push_back(center_x-0.12);
    x.push_back(center_x+0.12);
    y.push_back(center_y-0.12);
    y.push_back(center_y+0.12);
    fermatSpiral1->setData(x, y);
    ui->plot->replot();




}

void MainWindow::mapDraw()
{

//    find the robot position


   R.setX(0);
   R.setY(0);

//   dont modify real map
   plan->updatemap(map);

//    modify the local map
   map.push_front(QPoint(plan->lim.xmax,plan->lim.ymin));

//    left most point
   QPoint left(plan->lim.xmin,plan->lim.ymin);
   QPoint right(plan->lim.xmax,plan->lim.ymin);
   QPoint start=R;
   QVector<QPoint>blueMap;

   blueMap.push_back(left);

   blueMap.push_back(start);

   blueMap.push_back(right);

   usermap blueline=map2vector(blueMap);

//     connect curve by green color
   usermap slam=map2vector(map);
   ui->plot->graph(0)->setData(blueline.x, blueline.y);
   ui->plot->graph(1)->setData(slam.x, slam.y);// green line for map
   ui->plot->graph(1)->rescaleAxes(true);

   ui->plot->replot();


}




void MainWindow::slot_cloudread(localmap m){


//    map.clear();
    foreach(QPoint p, m.map){
        map.push_back(p);
    }

    scriptCount++;
    if(scriptCount%50==0){
        qDebug()<<map.size();
//        mapDraw();
    }

}

 void MainWindow::on_findPath_clicked()
 {
     pointDraw();
     searchSpace();
 }
















