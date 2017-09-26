/*
 * Copyright 2017 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

    #ifndef THREAD_H
    #define THREAD_H
     
    #include <QThread>
    #include "ui_counterDlg.h"
     
    class Thread : public QThread
    {
        Q_OBJECT
    public:
        explicit Thread(QObject *parent = 0);
//          virtual ~Thread();
       
    private:
        bool works;
        
    signals:
        void WorkDone();
     
    public slots:
        void run();
        void startStop();
        bool Works();
    };
     
    #endif // THREAD_H
