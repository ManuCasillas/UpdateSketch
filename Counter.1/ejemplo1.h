#ifndef ejemplo1_H
#define ejemplo1_H

#include <QtGui>
#include <QTimer>
#include "ui_counterDlg.h"




// namespace Ui {
//     class ejemplo1;
// }


class ejemplo1 : public QWidget, public Ui_Counter
{
Q_OBJECT
public:
   
    ejemplo1();
    virtual ~ejemplo1();
    
private:
    
     int cont = 0;
     QTimer timer;
    
    
public slots:
	void doButton();
    void doTime();
    
};

#endif // ejemplo1_H
