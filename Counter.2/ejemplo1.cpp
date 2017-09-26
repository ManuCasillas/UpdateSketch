#include "ejemplo1.h"


ejemplo1::ejemplo1(): Ui_Counter() 
{
	setupUi(this);
    show();
    
    cont = 0;

    connect(&thread,SIGNAL(WorkDone()),this,SLOT(doTime()));    
    connect(stopButton, SIGNAL(clicked(bool)), this,SLOT(doButton()));
    connect(reStartB, SIGNAL(clicked(bool)), this,SLOT(doRestart()));
    
    thread.start();
        
}

ejemplo1::~ejemplo1()
{}

void ejemplo1::doButton()
{
    
    if(thread.Works()){
        thread.startStop();
        stopButton->setText("START");
        
    }else{
        thread.startStop();
        stopButton->setText("STOP");
       
        
    }
    
}

void ejemplo1::doTime()
{
//    qDebug() << "Cont++:";
   cont++;
   lcdNumber->display(cont);
   

   
}

void ejemplo1::doRestart()
{
//    qDebug() << "Restart:";
   cont = 0;
   lcdNumber->display(cont);
   
    
}





