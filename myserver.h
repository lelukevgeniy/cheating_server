#ifndef MYSERVER
#define MYSERVER

#include <QWidget>
#include <QtWidgets>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QFile>

class MyServer : public QWidget {

    Q_OBJECT

private:

    QSqlDatabase sdb;
    QTcpServer* tcpServer;
    quint32 nextBlockSize;
    quint32 version;

private:

    void sendArrayToClient(QTcpSocket* pSocket, QList<QString> lst_str);
    void sendImageToClient(QTcpSocket* pSocket, QImage);
    void sendCurrVers(QTcpSocket* pSocket);

    QList<QString> getSubjects();
    QList<QString> getThemes(QString subj_nm);
    QList<QString> getPictures(QString subj_nm, QString thm_nm);
    QImage getImage(QString img_nm);

    void putSubject(QString subj_nm);
    void putTheme(QString subj_nm, QString thm_nm);
    quint32 putImage(QImage img, QString subj_nm, QString thm_nm,QString imgExt);

    void deleteSubject(QString subj_nm);
    void deleteTheme(QString subj_nm, QString thm_nm);
    void deleteImage(QString img_nm);

    void swapImage(QImage new_img, QString old_img_nm);

public:

    MyServer(int nPort, QWidget* pwgt = 0);

public slots:

    void slotNewConnection();

    void slotReadClient();
};


#endif // MYSERVER

