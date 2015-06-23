#include "stack.h"


//http://stackoverflow.com/questions/11039594/qt-passing-signal-containing-2d-array-from-one-thread-to-another
int main(int argc, char *argv[])
{
    ros::init(argc,argv,"ardrone_gui");
    
	// QT
    QApplication a(argc, argv);
    qRegisterMetaType<localmap>("localmap");
    MainWindow w;
    w.show();

    return a.exec();
}

