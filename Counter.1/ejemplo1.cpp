#include "ejemplo1.h"
//   #include "ui_counterDlg.h"
// #include <ui_counterDlg.h>


ejemplo1::ejemplo1(): Ui_Counter() 
{
    
	setupUi(this);
    
    connect(&timer,SIGNAL(timeout()),this,SLOT(doTime()));
    timer.start(60);
    
    connect(stopButton, SIGNAL(clicked(bool)), this, SLOT(doButton()) );
    
	show();
}

ejemplo1::~ejemplo1()
{}

void ejemplo1::doButton()
{

   cont = 0;
   lcdNumber->display(cont);
   
}

void ejemplo1::doTime()
{
    
  cont++;
   lcdNumber->display(cont);
    
}





