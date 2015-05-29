#ifndef EXECWINDOW_H
#define EXECWINDOW_H

#include <QtGui/QDialog>
#include <QProcess>
#include <list>

namespace Ui {
    class execWindow;
}

class execCmd
{
public:
    QString         title;
    QString         desc;
    QString         cmd;
    QStringList     params;

    execCmd(const QString& _title,const QString& _desc,const QString& _cmd,const QStringList& _params):
            title(_title),desc(_desc),cmd(_cmd),params(_params) {}
};

class execWindow : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(execWindow)
public:
    explicit execWindow(QWidget *parent, const QString& info=QString());
    virtual ~execWindow();

    std::list<execCmd> batch;

    void run(void);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent* e);

private slots:
    void on_pushButton_2_clicked();
    void onStarted();
    void onError(QProcess::ProcessError e);
    void onFinished(int exitcode);
    void onDataReady();


private:
    Ui::execWindow *m_ui;
    QProcess *m_proc;

    time_t beg_time;

    void NextCmd();
    void unescString(const char* buf,int len,QString& s);
};

#endif // EXECWINDOW_H
