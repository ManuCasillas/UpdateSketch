#ifndef ejemplo1_H
#define ejemplo1_H

#include <QtGui>
// #include <QTimer>
#include <thread>
#include <condition_variable>
#include <functional>
#include <unistd.h>
#include "ui_counterDlg.h"
#include "Thread.h"



class ejemplo1 : public QWidget, public Ui_Counter
{
Q_OBJECT
public:

    ejemplo1();
    ~ejemplo1();
    

private:
    int cont;
    Thread thread;
    std::thread myThread;
    std::condition_variable cv;
    std::mutex mutex;
    
    
public slots:
	void doButton();
    void doTime();
    void doRestart();
    
};

#endif // ejemplo1_H
